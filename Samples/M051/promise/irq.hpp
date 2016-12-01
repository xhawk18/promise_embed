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
	Defer defer_;
	irq_x(const Defer &Defer)
		: defer_(Defer){
	}

	static void wait(pm_list *irq_list, const Defer &defer){
		irq_x *irq = new(pm_allocator::template obtain<irq_x>()) irq_x(defer);
		pm_list *list = &pm_memory_pool_buf_header::from_ptr(irq)->list_;
		pm_allocator::add_ref(irq);
		irq_list->attach(list);
	}
	
	static void post(pm_list *irq_list){
		pm_list *next = irq_list->next();
		if(irq_list != next){
			irq_list->detach();
			pm_list *ready = irq_x::get_ready_list();
			ready->attach(next);
		}
	}
	
	static void run(){
		pm_list list;
		irq_disable();
		pm_list *ready = get_ready_list();
		pm_list *next = ready->next();
		if(ready != next){
			ready->detach();
			list.attach(next);
		}
		irq_enable();
		
		pm_list *node;
		pm_list *node_next;
		for(node = list.next(); node != &list; node = node_next){
			irq_x *irq = reinterpret_cast<irq_x *>(pm_memory_pool_buf_header::to_ptr(node));
			Defer defer = irq->defer_;
			node_next = node->next();
			node->detach();
			pm_allocator::dec_ref(irq);
			
			defer.resolve();
		}		
	}
	
	static inline pm_list *get_ready_list(){
		static pm_list *list = nullptr;
		if(list == nullptr){
			list = new
#ifdef PM_EMBED_STACK
                (pm_stack::allocate(sizeof(*list)))
#endif
                pm_list();
		}
		return list;
	}
	
};

template<int IRQ>
struct irq{
	static pm_list *get_waiting_list(){
		static pm_list *list = nullptr;
		if(list == nullptr){
			list = new
#ifdef PM_EMBED_STACK
                (pm_stack::allocate(sizeof(*list)))
#endif
                pm_list();
		}
		return list;
	}
	
	static void wait(const Defer &defer){
		irq_x::wait(get_waiting_list(), defer);
	}
	
	static void post(){
		irq_x::post(get_waiting_list());
	}
};

	
}

#endif
