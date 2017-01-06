#include <stdio.h>
#include <string>
#include "stm32f30x.h"
#include "promise.hpp"

/********************************************/
/* Functions required by promise library    */
/********************************************/
using namespace promise;

extern "C" {
/* For debug usage */
uint32_t g_alloc_size = 0;
uint32_t g_stack_size = 0;
uint32_t g_promise_call_len = 0;

/* SysTick_Handler in stm32f30x_it.cpp
void SysTick_Handler(){
    pm_timer::increase_ticks();
}
*/

}/* extern "C" */

void pm_run_loop(){
    pm_timer::init_system(SystemCoreClock);
    while(true){
        pm_run();
        __WFE();
    }
}


/********************************************/
/* Use defined procedure                    */
/********************************************/
void LED_init(){
    /* Open LED GPIO for testing
       PB8     LED_A
       PB9     LED_B
     */
    GPIO_InitTypeDef        GPIO_InitStructure;

    /* GPIOA GPIOB Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* PB8, PB9 in output mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

inline void LED_A(int on){
    if(!on) GPIOB->BSRR = GPIO_Pin_8;
    else    GPIOB->BRR  = GPIO_Pin_8;
}

inline void LED_B(int on){
    if(!on) GPIOB->BSRR = GPIO_Pin_9;
    else    GPIOB->BRR  = GPIO_Pin_9;
}

Defer LED_A_blink(int count){
    if(count <= 0) return delay_ms(0);

    return delay_ms(500).then([]()->Defer {
        LED_A(1);
        return delay_ms(500);
    }).then([=]()->Defer {
        LED_A(0);
        return LED_A_blink(count - 1);
    });

}

void LED_A_blink_fast(){
    delay_ms(200).then([]()->Defer {
        LED_A(1);
        return delay_ms(200);
    }).then([](){
        LED_A(0);
        LED_A_blink_fast();
    });
}

void LED_B_blink_fast(){
    delay_ms(200).then([]()->Defer {
        LED_B(1);
        return delay_ms(200);
    }).then([](){
        LED_B(0);
        LED_B_blink_fast();
    });
}


int main(){

    /* LED A 闪5次，
       然后等待3秒钟,
       然后LED A 和 LED B 同时快闪 */
    LED_A_blink(5).then([](){
        return delay_s(3);
    }).then([](){
        LED_A_blink_fast();
        LED_B_blink_fast();
    });

    /* 实际闪烁会在pm_run_loop中运行 */
    pm_run_loop();
    return 0;
}

