#include <stdio.h>
#include <string>
#include "M051Series.h"
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

/* SysTick_Handler must be in extern "C" */
void SysTick_Handler(){
    pm_timer::increase_ticks();
}

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
       P3.6    LED_A
       P3.7    LED_B
     */
    _GPIO_SET_PIN_MODE(P3, 6, GPIO_PMD_OUTPUT);
    _GPIO_SET_PIN_MODE(P3, 7, GPIO_PMD_OUTPUT);
}

inline void LED_A(int on){
    if(on) P3->DOUT |= BIT6;
    else   P3->DOUT &= ~BIT6;
}

inline void LED_B(int on){
    if(on) P3->DOUT |= BIT7;
    else   P3->DOUT &= ~BIT7;
}

//LED A 闪烁count次
Defer LED_A_blink(int count){
    if(count <= 0) return delay_ms(0);

    return delay_ms(500)                //等0.5秒
    .then([]()->Defer {
        LED_A(1);                       //LED A 打开
        return delay_ms(500);           //等0.5秒
    }).then([=]()->Defer {
        LED_A(0);                       //LED A 关闭
        return LED_A_blink(count - 1);  //再次闪烁
    });

}

//LED A 快速不断闪烁
void LED_A_blink_fast(){
    delay_ms(200).then([]()->Defer {    //等0.2秒
        LED_A(1);                       //LED A 打开
        return delay_ms(200);           //等0.2秒
    }).then([](){
        LED_A(0);                       //LED A 关闭
        LED_A_blink_fast();             //继续闪烁
    });
}

//LED B 快速不断闪烁
void LED_B_blink_fast(){
    delay_ms(200).then([]()->Defer {    //等0.2秒
        LED_B(1);                       //LED B 打开
        return delay_ms(200);           //等0.2秒
    }).then([](){
        LED_B(0);                       //LED B 关闭
        LED_B_blink_fast();             //继续闪烁
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

