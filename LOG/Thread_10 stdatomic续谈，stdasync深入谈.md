# Thread_10 std::atomic续谈，std::async深入谈

## 一  原子操作std::atomic续谈

一般atomic原子操作，针对++,--,+=，&=，|= 是支持的，其他的可能不可以

```c++
std::atomic<int> g_mycount = 0;
void mythread()
{
	for (int i = 0; i < 10000000; i++)
	{
		//g_mycount++; //对应的操作是原子操作（不会被打断）
		//g_mycount += 1;
		g_mycount = g_mycount + 1; //结果不对
	}
}

int main()
{
 
	std::thread mytobj1(mythread);
	std::thread mytobj2(mythread);
	mytobj1.join();
	mytobj2.join();
	return 0;
}
```

## 二 std::async深入谈

### 2.1 std::async参数详述，async用来创建一个 异步任务

```c++
std::cout << "main start " << "thread_id:" << std::this_thread::get_id() << std::endl;
	//std::future<int> result = std::async(mythread);
	//std::future<int> result = std::async(std::launch::deferred, mythread); //deferred延迟调用，并且不创建新线程，延迟到future对象调用.get()或者.wait()的时候才执行mythread()
	//std::future<int> result = std::async(std::launch::async, mythread);
	std::future<int> result = std::async(std::launch::deferred|std::launch::async, mythread);
	std::cout << result.get() << std::endl;
```

**std::launch::deferred**【延迟调用】，以及**std::launch::async**【强制创建一个线程】

**std::thread()** 如果系统资源紧张，那么可能创建线程就会失败，那么执行std::thread()时，整个程序就可能崩溃

**std::async()**我们一般不叫创建线程(解释async能够创建线程)，我们一般叫它创建 一个异步任务

std::async和std::thread最明显的不同，就是**async有时候不创建新线程**



**(a)如果你用std::launch::deferred来调用async会怎么样?**

 deferred延迟调用，并且不创建新线程，延迟到future对象调用.get()或者.wait()的时候才执行mythread()

**(b)std::launch::async**

强制这个异步任务在新线程上执行，这意味着，系统必须要创建出新线程来运行mythread()

**(c)std::launch::deferred|std::launch::async**

这里这个 | ：意味着调用async的行为可能是“创建新线程并立即执行”或者 “没有创建新线程并且延迟到调用result.get()才开始执行任务入库函数”，两者居其一

**(d)不带额外参数std::async(mythread)**

默认和(c)完全一样，系统会自行决定是异步(创建新线程)还是同步(不创建新线程)方式运行



系统如何决定是 异步(创建新线程)还是同步(不创建新线程)方式运行?

### 2.2 std::async和std::thread的区别

**取返回值**

std::thread 创建线程的方式，如果线程返回值，你想拿到这个值不容易

std::async 创建异步任务，可能创建也可能不创建线程，并且async调用方法很容易拿到线程入口的返回值

**由于系统资源限制**

(1)如果用std::thread创建的线程太多，则可能创建失败，系统报告异常，崩溃

(2)如果用std::async，一般就不会报异常，不会崩溃，因为，如果系统资源紧张导致无法创建新线程的时候，async这种不加额外参数的调用，就不会创建新线程，而是后续调用了result.get()来请求结果

如果你强制std::async一定要创建新线程，那么就必须使用std::launch::async，承受的带价就是系统资源紧张时，程序崩溃叭

经验：一个程序里，线程是数量不要超过100-200



### 2.3 std::async不确定性问题的解决

不加额外参数的std::async调用，让系统自行决定是否创建新线程

```c++
std::future<int> result = std::async(mythread); //想判断async到底有没有创建新线程立即执行还是
	
std::future_status status = result.wait_for(std::chrono::seconds(0));
//std::future_status status = result.wait_for(0s);
if (status == std::future_status::deferred) //延迟
{
    //线程被延迟执行了（系统资源紧张了，采用std::launch::deferred策略了）
    std::cout << "线程被延迟执行" << std::endl;
    std::cout << result.get() << std::endl;
}
else
{
    //任务没有被推迟，已经开始运行了，线程被创建了
    if (status == std::future_status::ready)
    {
        //线程返回成功
        std::cout << "线程成功执行完毕，返回" << std::endl;
        std::cout << result.get() << std::endl;
    }
    else if (status == std::future_status::timeout) //超时：我想等待你1秒，希望你返回，你没有返回，那么status=timeout
    {
        //超时：表示线程还没执行完
        std::cout << "超时，线程还没执行完" << std::endl;
        std::cout << result.get() << std::endl;
    }
}
```

