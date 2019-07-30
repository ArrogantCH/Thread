# Thread_8 async、future、packaged_task、promise

## 一 std::async、std::future创建后台任务并返回值

希望线程返回一个结果

std::async 是个函数模版，用来启动一个异步任务，启动起来一个异步任务后，返回一个std::future对象，std::future是个类模版

**启动一个“异步任务”**：自动创建一个线程并开始执行对应得线程入口函数，它返回一个std::future对象

这个std::future对象里面就含有线程入口函数所返回的结果(线程返回的结果)，我们就可以调用future对象的成员函数get()来获取结果

"future"：将来的意思，有人也称呼std::future提供了一种访问异步操作结果的机制

future(对象)会保存一个值，在将来的某个时刻可以拿到

```c++
int mythread()
{
	std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
	std::chrono::milliseconds dura(5000);
	std::this_thread::sleep_for(dura);
	std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
	return 5;
}

int main()
{
	//下列程序通过std::future对象的get()成员函数等待线程执行结束返回结果
	//这个get()函数很牛，不拿到值就卡在这里等待拿值
	std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;
	std::future<int> result = std::async(mythread); //绑定关系，流程不卡在这里

	std::cout << "continue... " <<  std::endl;
	int def = 0;
	//std::cout << result.get() << std::endl; //卡在这里等待mythread执行完
	//std::cout << result.get() << std::endl; //get()只能调一次

	result.wait(); //等待线程返回，本身并不返回结果

	std::cout << " I Love China"; 

	return 0;
}
```



我们通过额外向std::async()传递一个参数，该参数类型是std::launch类型（枚举类型），来达到一些特殊的目的

**(a)std::launch::deferred**

表示线程入口函数调用被延迟到std::future等wait()或者get()函数调用时才执行

**那如果wait()或者get()函数没有调用**，那么线程会执行吗？ 没执行，实际上，根本没创建

 std::launch::deferred: 延迟调用，并且没有创建新线程，**是在主线程中调用的线程入口函数**

**(b)std::launch::async**

在调用async函数的时候就开始创建线程

async()函数，默认就用的是std::launch::async标记

```c++
class A
{
public:
	int mythread(int mypar)
	{
		std::cout << mypar << std::endl;
		std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
		std::chrono::milliseconds dura(5000);
		std::this_thread::sleep_for(dura);
		std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
		return 5;
	}
};


int main()
{
	

	A a;
	int tmppar = 12;
	std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;
	//std::future<int> result = std::async(&A::mythread, &a, tmppar); //第二个参数是对象引用，才能保证线程里面用的是一个对象

	//std::future<int> result = std::async(std::launch::deferred, &A::mythread, &a, tmppar);
	std::future<int> result = std::async(std::launch::async, &A::mythread, &a, tmppar);

	std::cout << "continue... " <<  std::endl;
	int def = 0;
	//std::cout << result.get() << std::endl; //卡在这里等待mythread执行完
	//std::cout << result.get() << std::endl; //get()只能调一次

	result.wait(); //等待线程返回，本身并不返回结果

	std::cout << " I Love China"; 

	return 0;
}

```

## 二 std::package_task: 打包任务，把任务包装起来

是个类模版，它等模版参数是 各种可调用对象

通过std::package_task 来把各种可调用对象包装起来，方便将来作为线程入口函数来调用

```c++
int mythread(int mypar)
{
	std::cout << mypar << std::endl;
	std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
	std::chrono::milliseconds dura(5000);
	std::this_thread::sleep_for(dura);
	std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
	return 5;
}


int main()
{

	std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;
	std::packaged_task<int(int)> mypt(mythread); //我们把函数mythread通过package_task包装起来


	std::thread t1(std::ref(mypt), 1); //线程直接开始执行，第二个参数 作为线程入口函数的参数
	t1.join(); //等待线程执行完毕
	std::future<int> result = mypt.get_future();
	std::cout << result.get() << std::endl; //卡在这里等待mythread执行完
	
	return 0;
}

```

**包装lambda表达式**

```c++
std::packaged_task<int(int)> mypt([](int mypar) {
		std::cout << mypar << std::endl;
		std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
		std::chrono::milliseconds dura(5000);
		std::this_thread::sleep_for(dura);
		std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
		return 5;
	});
```

package_task包装起来的可调用对象还可以直接调用，所以从这个角度来将，**package_task对象，也是一个可调用对象**

```c++
mypt(105); //直接调用
std::future<int> result = mypt.get_future();
std::cout << result.get() << std::endl;
```

代码的编写方式和应用方式有很多，下面举个容器的例子叭

```c++
std::vector<std::packaged_task<int(int)>> mytasks;
int main()
{

	std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;

	//包装lambda表达式
	std::packaged_task<int(int)> mypt([](int mypar) {
		std::cout << mypar << std::endl;
		std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
		std::chrono::milliseconds dura(5000);
		std::this_thread::sleep_for(dura);
		std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
		return 5;
	});

	mytasks.push_back(std::move(mypt)); //容器，这里用了移动语义，入进去后mypt为空

	std::packaged_task<int(int)> mypt2;
	auto iter = mytasks.begin();
	mypt2 = std::move(*iter); //移动语义,
    //这里需要特别注意，虽然使用了移动语义，但是在vector里，还存在一个null的package_task对象呢！！！！！！！！
	mytasks.erase(iter); //删除第一个元素，迭代已经失效了，所以后续代码不可以再使用iter
	mypt2(123);
	std::future<int> result = mypt.get_future();
	std::cout << result.get() << std::endl;

	return 0;
}
```

## 三 std::promise 类模版

我们能够在某个线程中给它赋值，然后我们可以在其他线程汇总，把这个值取出来用

总结： 通过promise保存一个值，在将来某个时刻我们通过把一个future绑定到这个promise上来得到这个绑定的值

```c++
void mythread(std::promise<int> &tmpp, int calc) //注意第一个参数
{
	//做一系列复杂操作
	calc++;
	calc += 10;
	//比如还有一些其他的算法，整整花费了5秒钟
	std::chrono::seconds ss(5);
	std::this_thread::sleep_for(ss);

	//终于计算出结果了
	int result = calc;
	tmpp.set_value(result); //结果我保存到了tempp这个对象中
}

void mythread2(std::future<int> &tmpf)
{
	auto result = tmpf.get();
	std::cout << result << std::endl;

}

int main()
{
	std::promise<int> myprom; //声明一个std::promise对象，保存的值类型为int
	std::thread t1(mythread, std::ref(myprom), 180);
	t1.join();

	//获得结果值
	std::future<int> ful = myprom.get_future();
    
	//std::cout << ful.get() << std::endl; //get只能调用1次

	std::thread t2(mythread2, std::ref(ful));
	t2.join();

	return 0;
}
```

