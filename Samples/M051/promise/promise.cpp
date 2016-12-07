#include <stdio.h>
#include <string>
#include "M051Series.h"         // For M051 series
#include "promise.hpp"

using namespace promise;

#define output_func_name() do{ printf("in function %s, line %d\n", __func__, __LINE__); } while(0)

void test1() {
    output_func_name();
}

int test2() {
    output_func_name();
    return 5;
}

void test3() {
    output_func_name();
}

Defer run(Defer &next){

    return newPromise([](Defer d){
        output_func_name();
        d.resolve();
    }).then([]() {
        output_func_name();
    }).then([](){
        output_func_name();
    }).then([&next]()->Defer{
        output_func_name();
        next = newPromise([](Defer d) {
            output_func_name();
        });
        //Will call next.resole() or next.reject() later
        return next;
    }).then([]() {
        output_func_name();
    }).fail([](){
        output_func_name();
    }).then(test1)
    .then(test2)
    .then(test3)
    .always([]() {
        output_func_name();
    });
}

Defer run2(Defer &next){

    return newPromise([](Defer d){
        output_func_name();
        d.resolve();
    }).then([]() {
        output_func_name();
    }).then([](){
        output_func_name();
    }).then([&next]()->Defer{
        output_func_name();
        next = newPromise([](Defer d) {
            output_func_name();
        });
        //Will call next.resole() or next.reject() later
        return next;
    }).then([]() {
        output_func_name();
    }).fail([](){
        output_func_name();
    }).then(test1)
    .then(test2)
    .then(test3)
    .always([]() {
        output_func_name();
    });
}

void pm_run_loop(){
#define TT_SYSTICK_CLOCK        22118400
    pm_timer::init_system(TT_SYSTICK_CLOCK);
    while(true){
        __WFE();
        pm_timer::run();
        irq_x::run();
        defer_list::run();
    }
}


extern "C"{

inline void LED_A(int on){
    if(on) P3->DOUT |= BIT6;
    else   P3->DOUT &= ~BIT6;
}

inline void LED_B(int on){
    if(on) P3->DOUT |= BIT7;
    else   P3->DOUT &= ~BIT7;    
}

Defer dd;

void test_0(int n){
    int *n_ = pm_new<int>(n);
    delay_ms(2000).then([]()->Defer {
        LED_B(1);
        return delay_ms(2000);
    }).then([n_](){
        LED_B(0);
        if(*n_ > 0)
            test_0(*n_ - 1);
        else
            kill_timer(dd);
        pm_delete(n_);
    });
}


void test_1(){
    dd = delay_ms(300).then([]()->Defer {
        LED_A(1);
        dd = delay_ms(300);
        return dd;
    }).then([]() {
        LED_A(0);
        test_1();
    });
}

void test_irq(){
    newPromise([](Defer d){
        LED_A(1);
        irq_disable();
        irq<SysTick_IRQn>::wait(d);
        irq_enable();
    }).then([]() {
        return newPromise([](Defer d){
            LED_A(0);
            irq_disable();
            irq<SysTick_IRQn>::wait(d);
            irq_enable();
        });
    }).then([]() {
        test_irq();
    });
}

void SysTick_Handler(){
    timer_global *global = pm_timer::get_global();
    global->current_ticks_++;
    
    if(global->current_ticks_ % 1024 == 0)
        irq<SysTick_IRQn>::post();
}
    
void main_cpp(){
    //test_0(2);
    test_1();
    
    //test_irq();
    pm_run_loop();
}

}
