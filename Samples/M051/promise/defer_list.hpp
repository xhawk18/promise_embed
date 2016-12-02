#ifndef INC_DEFER_LIST_H_
#define INC_DEFER_LIST_H_

namespace promise{
	
struct defer_list{
	static pm_list *get_list(){
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

	static void attach(pm_list *list, const Defer &defer){
		Defer *defer_ = new(pm_allocator::template obtain<Defer>()) Defer(defer);
		pm_list *node = &pm_memory_pool_buf_header::from_ptr(defer_)->list_;
		list->attach(node);
		pm_allocator::add_ref(defer_);
	}
	
	static void attach(pm_list *list, pm_list *other){
		pm_list *next = other->next();
		if(next != other){
			other->detach();
			list->attach(next);
		}
	}
	
	static void run(pm_list *list){
		pm_list *node;
		pm_list *node_next;
		for(node = list->next(); node != list; node = node_next){
			Defer *defer = reinterpret_cast<Defer *>(pm_memory_pool_buf_header::to_ptr(node));
			node_next = node->next();
			node->detach();
			Defer defer_ = *defer;
			pm_allocator::dec_ref(defer);
			
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
};

}

#endif
