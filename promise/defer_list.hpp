#pragma once
#ifndef INC_DEFER_LIST_H_
#define INC_DEFER_LIST_H_

namespace promise{

struct defer_list{
    static void attach(pm_list *list, const Defer &defer){
        Defer *defer_ = pm_new<Defer>(defer);
        pm_list *node = &pm_memory_pool_buf_header::from_ptr(defer_)->list_;
        list->attach(node);
    }

    static void attach(pm_list *list, pm_list *other){
        pm_list *next = other->next();
        if(next != other){
#ifdef PM_DEBUG
            for(pm_list *node = next; node != other; node = node->next()){
                pm_memory_pool_buf_header *header = pm_container_of(node, &pm_memory_pool_buf_header::list_);
                pm_assert(header->ref_count_ > 0);
            }
#endif
            other->detach();
            list->attach(next);
        }
    }
    
    static void remove(pm_list *list, const Defer &defer){
        pm_list *node = list->next();
        while(node != list){
            Defer *defer_ = reinterpret_cast<Defer *>(pm_memory_pool_buf_header::to_ptr(node));
            pm_list *node_next = node->next();
            
            if(*defer_ == defer){
                node->detach();
                pm_delete(defer_);
            }
            node = node_next;
        }
    }

    static void run(pm_list *list){
        while(!list->empty()){
            pm_list *node = list->next();
            Defer *defer = reinterpret_cast<Defer *>(pm_memory_pool_buf_header::to_ptr(node));
            node->detach();
            Defer defer_ = *defer;
            pm_delete(defer);

            defer_.resolve();
        }
    }

    static inline void attach(const Defer &defer){
        attach(get_list(), defer);
    }
    static inline void attach(pm_list *other){
        attach(get_list(), other);
    }
    static void run(){
        run(get_list());
    }

    static inline void remove(const Defer &defer){
        remove(get_list(), defer);
    }
private:
    static pm_list *get_list(){
        static pm_list *list = nullptr;
        if(list == nullptr)
            list = pm_stack_new<pm_list>();
        return list;
    }
};

}

#endif
