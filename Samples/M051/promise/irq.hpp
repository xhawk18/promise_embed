#pragma once
#ifndef INC_IRQ_HPP_
#define INC_IRQ_HPP_

#include "promise_min.hpp"

namespace promise{

static inline void irq_disable(){
    __set_PRIMASK(1);
}

static inline void irq_enable(){
    __set_PRIMASK(0);
}

struct irq_x{
    static void wait(pm_list *irq_list, const Defer &defer){
        defer_list::attach(irq_list, defer);
    }

    static void post(pm_list *irq_list){
        pm_list *ready = irq_x::get_ready_list();
        defer_list::attach(ready, irq_list);
    }

    static void run(){
        pm_list list;
        irq_disable();
        pm_list *ready = get_ready_list();
        defer_list::attach(&list, ready);
        irq_enable();

        defer_list::attach(&list);
    }

private:
    static inline pm_list *get_ready_list(){
        static pm_list *list = nullptr;
        if(list == nullptr)
            list = pm_stack_new<pm_list>();
        return list;
    }

};

template<int IRQ>
struct irq{
    static void wait(const Defer &defer){
        irq_x::wait(get_waiting_list(), defer);
    }

    static void post(){
        irq_x::post(get_waiting_list());
    }

private:
    static pm_list *get_waiting_list(){
        static pm_list *list = nullptr;
        if(list == nullptr)
            list = pm_stack_new<pm_list>();
        return list;
    }
};


}

#endif
