# C++ promise/A+ library for embedded.

<!-- TOC -->

- [C++ promise/A+ library for embedded.](#c-promisea-library-for-embedded)
    - [什么是 promise-embed ?](#什么是-promise-embed-)
    - [例子](#例子)
        - [编译器要求](#编译器要求)
        - [例子代码](#例子代码)
    - [Global functions](#global-functions)
        - [Defer newPromise(FUNC func);](#defer-newpromisefunc-func)
        - [Defer resolve();](#defer-resolve)
        - [Defer reject();](#defer-reject)
        - [Defer doWhile(FUNC func);](#defer-dowhilefunc-func)
    - [Class Defer - type of promise object.](#class-defer---type-of-promise-object)
        - [Defer::resolve(const RET_ARG... &ret_arg);](#deferresolveconst-ret_arg-ret_arg)
        - [Defer::reject(const RET_ARG... &ret_arg);](#deferrejectconst-ret_arg-ret_arg)
        - [Defer::then(FUNC_ON_RESOLVED on_resolved, FUNC_ON_REJECTED on_rejected)](#deferthenfunc_on_resolved-on_resolved-func_on_rejected-on_rejected)
        - [Defer::then(FUNC_ON_RESOLVED on_resolved)](#deferthenfunc_on_resolved-on_resolved)
        - [Defer::fail(FUNC_ON_REJECTED on_rejected)](#deferfailfunc_on_rejected-on_rejected)
        - [Defer::finally(FUNC_ON_FINALLY on_finally)](#deferfinallyfunc_on_finally-on_finally)
        - [Defer::always(FUNC_ON_ALWAYS on_always)](#deferalwaysfunc_on_always-on_always)
    - [Timers](#timers)
        - [Defer yeild();](#defer-yeild)
        - [Defer delay_ms(uint32_t msec);](#defer-delay_msuint32_t-msec)
        - [Defer delay_s(uint32_t sec);](#defer-delay_suint32_t-sec)
        - [void kill_timer(Defer &defer);](#void-kill_timerdefer-defer)
    - [Handle CPU interrupt](#handle-cpu-interrupt)
        - [void irq_disable()](#void-irq_disable)
        - [void irq_enable()](#void-irq_enable)
        - [irq<IRQ_NUMBER>::wait(const Defer &defer)](#irqirq_numberwaitconst-defer-defer)
        - [irq<IRQ_NUMBER>::post()](#irqirq_numberpost)
    - [And more ...](#and-more-)
        - [about c++ exceptions](#about-c-exceptions)
        - [copy the promise object](#copy-the-promise-object)

<!-- /TOC -->

## 什么是 promise-embed ?

Promise-embed为嵌入式开发提供了多任务支持，无需任何的操作系统，无需多线程。

和promise-cpp一样，promise-embed是仿照Javascript Promise/A+标准的c++版本实现。同时，它精简了resolve/reject函数的实现，可以运行与内存极其受限的环境中。例如运行于Cortex-M0/M3芯片里。
  
Promise-embed和promise-cpp一样，利用c++11的能力，通过一个任务循环，提供了单线程多任务执行环境。

根据嵌入式芯片的特殊要求，promise-embed也提供了在任务中使用等待时间和中断的方法。


## 例子

* [M051](examples/M051/main.cpp): 在M051芯片里，同时运行两个LED闪烁任务的例子。

* [STM32F302](examples/STM32F302/main.cpp): 在STM32芯片里，同时运行两个LED闪烁任务的例子。

### 编译器要求

需要编译器支持c++11或更高版本，例如

Keil/ARMCC 5，需要右键点击cpp源文件，在"Misc Control"里加上编译选项 --cpp11

### 例子代码

这个例子显示了单线程里运行多个任务，控制两个LED的显示。

```cpp
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
```

## Global functions

### Defer newPromise(FUNC func);
Creates a new Defer object with a user-defined function.
The user-defined functions, used as parameters by newPromise, must have a parameter Defer d. 
for example --

```cpp
return newPromise([](Defer d){
})
```

### Defer resolve();
Returns a promise that is resolved with the given value.
for example --

```cpp
return resolve();
```

### Defer reject();
Returns a promise that is rejected with the given arguments.
for example --

```cpp
return reject();
```

### Defer doWhile(FUNC func);
"While loop" for promisied task.
A promise(Defer) object will passed as parameter when call func, which can be resolved to continue with the "while loop", or be rejected to break from the "while loop". 

for example --

```cpp
doWhile([](Defer d){
    // Add code here for your task in "while loop"
    
    // Call "d.resolve();" to continue with the "while loop",
    
    // or call "d.reject();" to break from the "while loop", in this case,
    // the returned promise object will be in rejected status.
});

```

## Class Defer - type of promise object.

class Defer is the type of promise object.

### Defer::resolve(const RET_ARG... &ret_arg);
Resolve the promise object with arguments, where you can put any number of ret_arg with any type.
(Please be noted that it is a method of Defer object, which is different from the global resolve function.)
for example --

```cpp
return newPromise([](Defer d){
    d.resolve();
})
```

### Defer::reject(const RET_ARG... &ret_arg);
Reject the promise object with arguments, where you can put any number of ret_arg with any type.
(Please be noted that it is a method of Defer object, which is different from the global reject function.)
for example --

```cpp
return newPromise([](Defer d){
    d.reject();
})
```

### Defer::then(FUNC_ON_RESOLVED on_resolved, FUNC_ON_REJECTED on_rejected)
Return the chaining promise object, where on_resolved is the function to be called when 
previous promise object calls function resolve, on_rejected is the function to be called
when previous promise object calls function reject.
for example --

```cpp
return newPromise([](Defer d){
    d.resolve();
}).then(

    /* function on_resolved */ [](){
        printf("resolved here\n");   //will print "resolved here"
    },

    /* function on_rejected */ [](){
        printf("rejected here\n");   //will NOT run to here in this code 
    }
);
```

### Defer::then(FUNC_ON_RESOLVED on_resolved)
Return the chaining promise object, where on_resolved is the function to be called when 
previous promise object calls function resolve.
for example --

```cpp
return newPromise([](Defer d){
    d.resolve();
}).then([](){
    printf("resolved here\n");   //will print "resolved here"
});
```

### Defer::fail(FUNC_ON_REJECTED on_rejected)
Return the chaining promise object, where on_rejected is the function to be called when
previous promise object calls function reject.

This function is usually named "catch" in most implements of Promise library. 
  https://www.promisejs.org/api/

In promise_cpp, function name "fail" is used instead of "catch", since "catch" is a keyword of c++.

for example --

```cpp
return newPromise([](Defer d){
    d.reject();
}).fail([](int err, string &str){
    printf("rejected here\n");   //will print "rejected here"
});
```

### Defer::finally(FUNC_ON_FINALLY on_finally)
Return the chaining promise object, where on_finally is the function to be called whenever
the previous promise object is be resolved or rejected.

The returned promise object will keeps the resolved/rejected state of current promise object.

for example --

```cpp
return newPromise([](Defer d){
    d.reject();
}).finally([](){
    printf("in finally\n");   //will print "in finally" here
});
```

### Defer::always(FUNC_ON_ALWAYS on_always)
Return the chaining promise object, where on_always is the function to be called whenever
the previous promise object is be resolved or rejected.

The returned promise object will be in resolved state whenever current promise object is
resolved or rejected.

for example --

```cpp
return newPromise([](Defer d){
    d.reject();
}).always([](){
    printf("in always\n");   //will print "in always" here
});
```

## Timers

### Defer yeild();
Yield current task and let other tasks hava an opportunity to run

for example --

```cpp
promise::resolve().then([]({
    return yeield();
}).then([](){
    //After yeild, come back to here
});
```

### Defer delay_ms(uint32_t msec);
Delay for milliseconds.

for example --

```cpp
delay_ms(500).then([](){
    //After 500 milliseconds, come back to here
});
```

### Defer delay_s(uint32_t sec);
Delay for seconds.

for example --

```cpp
delay_s(5).then([](){
    //After 5 seconds, come back to here
});
```

### void kill_timer(Defer &defer);
Kill a delaying timer.

for example --

```cpp
Delay timer;
(timer = delay_s(5)).then([](){
    //After 5 seconds, come back to here
}).fail([](){
    printf("timer was killed\n");   //Will print "timer was killed"
});

//After 1 second, kill the timer for delay_s(5)
delay_s(1).then([=](){
    kill_timer(timer);
})
```

## Handle CPU interrupt

### void irq_disable()
(In thread) Disable CPU interrupt

### void irq_enable()
(In thread) Disable CPU interrupt

### irq<IRQ_NUMBER>::wait(const Defer &defer)
(In thread) Wait until the irq event was post, and then set "defer" as resolved status.

### irq<IRQ_NUMBER>::post()
(In irq) Post the irq event.

for example, in irq handler --

```cpp
void EXTI0_IRQHandler(){
    if((EXTI_GetITStatus(EXTI_Line0) != RESET)){
        /* Clear the EXTI line 0 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line0);
        irq<EXTI0_IRQn>::post();    //Post irq event
    }
}
```

in thread --
```cpp
    newPromise([=](Defer &d){
        irq_disable();
        irq<USART1_IRQn>::wait(d);
        irq_enable();
    }).then([](){
        //Will run to here after irq<EXTI0_IRQn>::post() called. 
    });
```

## And more ...

### about c++ exceptions
Promise-embed does not support to catch exceptions.

### copy the promise object
To copy the promise object is allowed and effective, please do that when you need.

```cpp
Defer d = newPromise([](Defer d){});
Defer d1 = d;  //It's safe and effective
```

