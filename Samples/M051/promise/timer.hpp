#ifndef INC_TIMER_HPP_
#define INC_TIMER_HPP_

#include "promise_min.hpp"

#define TT_TICKS_PER_SECOND	1000

namespace promise{

struct timer_global {
    pm_list  timers_;
    pm_list  waked_timers_;
    volatile uint64_t current_ticks_;
    volatile uint64_t time_offset_;             /* tt_set_time() only set this value */	
	
/* Why TT_TICKS_DEVIDER use (4096*1024),
   to use this value, maximum of TT_TICKS_PER_SECOND_1 is (4096*1024),
   1000*TT_TICKS_PER_SECOND_1 < pow(2,32), and will not overflow in tt_ticks_to_msec
 */
#define TT_TICKS_DEVIDER	(4096*1024)
    volatile uint64_t TT_TICKS_PER_SECOND_1;    /*  TT_TICKS_DEVIDER/TT_TICKS_PER_SECOND for fast devision */
};

struct pm_timer {

	static inline timer_global *get_global(){
		static timer_global *global = nullptr;
		if(global == nullptr)
			global = new
#ifdef PM_EMBED_STACK
                (pm_stack::allocate(sizeof(*global)))
#endif
                timer_global();
		return global;
	}
	
	static void init_system(uint32_t systick_frequency){
		timer_global *global = pm_timer::get_global();
		global->TT_TICKS_PER_SECOND_1 = TT_TICKS_DEVIDER / TT_TICKS_PER_SECOND;
		SysTick_Config(systick_frequency / TT_TICKS_PER_SECOND);
	}
	
	static uint64_t get_time(){
		timer_global *global = pm_timer::get_global();
		uint64_t u64_ticks_offset = global->time_offset_ * TT_TICKS_PER_SECOND;
		uint64_t u64_sec = (global->current_ticks_ + u64_ticks_offset) / TT_TICKS_PER_SECOND;
		return u64_sec;
	}

	static uint64_t set_time(uint64_t new_time){
		timer_global *global = pm_timer::get_global();
		uint64_t rt = global->time_offset_;
		global->time_offset_ = new_time;
		return rt;
	}
	
	static uint32_t get_ticks(void){
		timer_global *global = pm_timer::get_global();
		return global->current_ticks_;
	}

	static uint32_t ticks_to_msec(uint32_t ticks){
		timer_global *global = pm_timer::get_global();
		uint64_t u64_msec = 1000 * (uint64_t)ticks * global->TT_TICKS_PER_SECOND_1 / TT_TICKS_DEVIDER;
		uint32_t msec = (u64_msec >  (uint64_t)~(uint32_t)0
			? ~(uint32_t)0 : (uint32_t)u64_msec);
		return msec;
	}

	static uint32_t msec_to_ticks(uint32_t msec){
		timer_global *global = pm_timer::get_global();
		uint64_t u64_ticks = TT_TICKS_PER_SECOND * (uint64_t)msec / 1000;
		uint32_t ticks = (u64_ticks > (uint64_t)(uint32_t)0xFFFFFFFF
			? (uint32_t)0xFFFFFFFF : (uint32_t)u64_ticks);

		return ticks;
	}
	
	static void wakeup(){
		timer_global *global = pm_timer::get_global();

		uint32_t current_ticks = pm_timer::get_ticks ();
		
		pm_list *node_next;
		pm_list *node;

		for(node = global->timers_.next(); node != &global->timers_; node = node_next){
			pm_memory_pool_buf_header *header = pm_container_of(node, &pm_memory_pool_buf_header::list_);
			pm_timer *timer = reinterpret_cast<pm_timer *>(pm_memory_pool_buf_header::to_ptr(header));

			int32_t ticks_to_wakeup = (int32_t)(timer->wakeup_ticks_ - current_ticks);
			node_next = node->next();

			if(ticks_to_wakeup <= 0)
				global->waked_timers_.move(node);
			else break;
		}
		
		for(node = global->waked_timers_.next(); node != &global->waked_timers_; node = node_next){
			pm_timer *timer = reinterpret_cast<pm_timer *>(pm_memory_pool_buf_header::to_ptr(node));
			Defer defer = timer->defer_;
			node_next = node->next();
			node->detach();
			pm_allocator::dec_ref(timer);
			
			defer.resolve();
		}
	}
	

	uint32_t wakeup_ticks_;
	Defer defer_;
	
	pm_timer(const Defer &defer)
		: defer_(defer){
	}
	
	void start2(uint32_t ticks){
		timer_global *global = pm_timer::get_global();
	
		uint32_t current_ticks = pm_timer::get_ticks ();
		wakeup_ticks_ = current_ticks + ticks;
	
		pm_list *node = global->timers_.next();
		for(; node != &global->timers_; node = node->next()){
			pm_memory_pool_buf_header *header = pm_container_of(node, &pm_memory_pool_buf_header::list_);
			pm_timer *timer = reinterpret_cast<pm_timer *>(pm_memory_pool_buf_header::to_ptr(header));
			int32_t ticks_to_wakeup = (int32_t)(timer->wakeup_ticks_ - current_ticks);
			if (ticks_to_wakeup > ticks)
				break;
		}

		pm_list *list = &pm_memory_pool_buf_header::from_ptr(this)->list_;
		if(!list->empty())
			list->detach();
		else
			pm_allocator::add_ref(this);
		node->attach(list);
	}

	void start(uint32_t msec){
		start2(msec_to_ticks(msec));
	}

	void stop(){
		pm_list *list = &pm_memory_pool_buf_header::from_ptr(this)->list_;
		if(!list->empty()){
			list->detach();
			pm_allocator::dec_ref(this);
		}
	}

	
};

inline Defer delay_ticks(uint32_t ticks) {
	return newPromise([&ticks](Defer d){
		pm_timer *timer = new(pm_allocator::template obtain<pm_timer>()) pm_timer(d);
		timer->start2(ticks);
	});
}

inline Defer delay_ms(uint32_t msec) {
	return delay_ticks(pm_timer::msec_to_ticks(msec));
}

inline Defer delay_s(uint32_t sec) {
	uint64_t u64_ticks = TT_TICKS_PER_SECOND * (uint64_t)sec;
	uint32_t sleep_ticks = (u64_ticks > (uint64_t)(uint32_t)0xFFFFFFFF
		? (uint32_t)0xFFFFFFFF : (uint32_t)u64_ticks);
	return delay_ticks(sleep_ticks);
}

}
#endif

