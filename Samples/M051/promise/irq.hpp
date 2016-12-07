#pragma once
#ifndef INC_IRQ_HPP_
#define INC_IRQ_HPP_

//#include "promise.hpp"

namespace promise{

static inline void irq_disable(){
    __set_PRIMASK(1);
}

static inline void irq_enable(){
    __set_PRIMASK(0);
}

struct irq_x{
    /* Called in thread, need call -- 
        irq_disable();
        //add user code ...
        wait(...);
        //add user code ...
        irq_enable();
      */
    static void wait(pm_list *irq_list, const Defer &defer){
        defer_list::attach(irq_list, defer);
    }

    /* Called in interrupt */
    static void post(pm_list *irq_list){
        irq_disable();
        pm_list *ready = irq_x::get_ready_list();
        defer_list::attach(ready, irq_list);
        irq_enable();
    }
    
    /* Called in thread */
    static void kill(pm_list *irq_list, Defer &defer){
        if(defer.operator->()){
            Defer pending;
            irq_disable();
            if(defer.operator->()){
                pending = defer.find_pending();
                if(pending.operator->()){
                    defer_list::remove(irq_list, pending);
                    pm_list *ready = get_ready_list();
                    defer_list::remove(ready, pending);
                }
            }
            irq_enable();
            
            if(pending.operator->()){
                defer_list::remove(pending);
                pending.reject();
            }

            defer.clear();
        }
    }

    static void run(){
        irq_disable();
        pm_list *ready = get_ready_list();
        defer_list::attach(ready);
        irq_enable();
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

    static void kill(Defer &defer){
        irq_x::kill(get_waiting_list(), defer);
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
