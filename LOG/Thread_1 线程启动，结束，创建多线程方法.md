## Thread_1 线程启动，结束，创建多线程方法

主线程从main()开始执行

整个进程是否执行完的标志是，主线程是否执行完，如果主线程执行完了，整个进程执行完毕

一般情况：如果其他子线程没有执行完毕，主线程执行完了，这些子线程也会被操作系统强行终止

一般结论：

如果想保持子线程运行状态的话，那么一定得让主线程保持运行状态

### 一 基本操作

#### 1.1 thread 

​		标准库的类

#### 1.2 join()

 		加入/汇合，说白就是阻塞，阻塞主线程，让主线程等待子线程执行完毕，然后子线程和主线程汇合

#### 1.3 detach() 

​		 尽量不要用

​		传统多线程程序主线程等待子线程执行完毕，然后再最后自己退出

​        detach: 分离，也就是主线程不和子线程汇合，主线程执行主线程，子线程执行子线程，互不影响

​         一旦detach后，与这个主线程相关联的thread对象就会失去与这个主线程的关联，此时这个子线程就会驻留在后台运行，这个子线程就相当于被C++运行库接管，当这个子线程完成后，由运行时库负责清理该线程相关程序

​         一旦detach()后的子线程，就不能再join()了

#### 1.4  joinable()

​		判断是否可以成功使用join()或者detach()

​		返回true：可以join()/detach()，否则返回false

​		 

### 二 其他创建线程的手法

#### 2.1 用类对象（可调用对象）

####  

```c++
class TA
{
public:
	int _mi;
	TA(int i) : _mi(i) {}

	// 需要重载这个operator //不能带参数，让这个类成为可调用对象
	void operator()()
	{
		std::cout << "类中子线程开始" << std::endl;
		//...

		//...
		std::cout << "类中子线程结束" << std::endl;

	}

};
```

​	 可能的疑问：一旦调用了detach()。那主线程执行结束了，这里用的ta对象还在吗？

​     这个对象实际上是被复制到线程中去，执行完主线程后，ta会被销毁，但是所复制的TA对象依旧存在

​     所以，只要你这个TA类对象理没有引用，没有指针，那么就不会产生问题

#### 2.2 lambda表达式

```c++
auto mylambdaThread = [] {

		std::cout << "lambda线程开始" << std::endl;
		//...

		//...
		std::cout << "lambda线程结束" << std::endl;
	};
```





#### 整体测试代码

```c++
// thread_1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <string>

class TA
{
public:
	int _mi;
	TA(int i) : _mi(i) {}

	// 需要重载这个operator //不能带参数，让这个类成为可调用对象
	void operator()()
	{
		std::cout << "类中子线程开始" << std::endl;
		//...

		//...
		std::cout << "类中子线程结束" << std::endl;

	}

};

void testThread()
{
	std::cout << "子线程开始" << std::endl;

	//...

	//...
	std::cout << "子线程结束" << std::endl;
}

int main()
{
	int a = 0;
	TA ta(a);

	auto mylambdaThread = [] {

		std::cout << "lambda线程开始" << std::endl;
		//...

		//...
		std::cout << "lambda线程结束" << std::endl;
	};

	std::thread threadObj(ta);


	threadObj.join();
	threadObj.detach();

	std::cout << "主线程1" << std::endl;
	std::cout << "主线程2" << std::endl;
	std::cout << "主线程3" << std::endl;
}


```

