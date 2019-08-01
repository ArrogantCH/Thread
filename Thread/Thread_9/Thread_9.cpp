// Thread_9.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <future>


//int mythread()
//{
//	std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
//	std::chrono::milliseconds dura(5000);
//	std::this_thread::sleep_for(dura);
//	std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
//	return 5;
//}

//int mythread(int mypar)
//{
//	std::cout << mypar << std::endl;
//	std::cout << "mythread() start " << "thread_id:" << std::this_thread::get_id() << std::endl;
//	std::chrono::milliseconds dura(5000);
//	std::this_thread::sleep_for(dura);
//	std::cout << "mythread() end " << "thread_id:" << std::this_thread::get_id() << std::endl;
//	return 5;
//}

//void mythread2(std::future<int> &tmpf)
//{
//	auto result = tmpf.get(); //只能获取一次
//								// 为什么第二次get这个future我们会得到一个异常，主要是因为get函数的设计，是一个移动语义
//								// 因为第一次get后，就将值移动到result里了，就变成了空，如果再get，就会异常
//	std::cout << result << std::endl;
//
//}

//void mythread2(std::shared_future<int> &tmpf)
//{
//	auto result = tmpf.get(); //只能获取一次
//								// 为什么第二次get这个future我们会得到一个异常，主要是因为get函数的设计，是一个移动语义
//								// 因为第一次get后，就将值移动到result里了，就变成了空，如果再get，就会异常
//	std::cout << result << std::endl;
//
//}


//std::mutex g_my_mutex;
//int g_mycount = 0;
//std::atomic<int> g_mycount = 0; //我们封装了一个类型为int的对象（值），我们可以像操作一个int类型变量一样来操作这个g_mycount
//
//void mythread()
//{
//	for (int i = 0; i < 10000000; i++)
//	{
//		//g_my_mutex.lock();  //7秒钟实现了2000万次加锁和解锁
//		//g_mycount++;
//		////...
//		////...
//		//g_my_mutex.unlock();
//		g_mycount++; //对应的操作是原子操作（不会被打断）
//	}
//}
//

std::atomic<bool> g_ifend = false; //线程退出标记， 这里是原子标记

void mythread()
{
	std::chrono::milliseconds dura(1000);
	while (!g_ifend)
	{
		//系统没要求线程退出
		std::this_thread::sleep_for(dura);
	}
}

int main()
{
	//一、std::future  get()函数转移数据
	//二、std::shared_future 类模版，get()函数复制数据
	//三、原子操作std::atomic
	// (3.1) 原子操作概念引出范例
	



	//std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;
	//std::packaged_task<int(int)> mypt(mythread); //我们把函数mythread通过package_task包装起来


	//std::thread t1(std::ref(mypt), 1); //线程直接开始执行，第二个参数 作为线程入口函数的参数
	//t1.join(); //等待线程执行完毕
	////std::future<int> result = mypt.get_future();


	////bool ifcanget = result.valid(); //true
	//////std::shared_future<int> result_s(std::move(result));
	////std::shared_future<int> result_s(result.share()); //执行完毕后，result_s里有值，而result里空了
	////ifcanget = result.valid(); //false

	//std::shared_future<int> result_s(mypt.get_future()); //通过get_future直接获取future对象
	//auto mythreadresult = result_s.get();
	//mythreadresult = result_s.get();

	////std::thread t2(mythread2, std::ref(result));
	//std::thread t2(mythread2, std::ref(result_s));
	//t2.join();


	//三、原子操作std::atomic
	// (3.1) 原子操作概念引出范例
	//  互斥量：多线程编程中，保护共享数据 
	// 有两个线程，对一个变量进行操作，一个线程读该变量值，另一个线程往这个变量中写值
	// int atomvalue = 5;
	// 读线程A
	// int tmpvalue = atomvalue; //atomvalue代表的是多个线程之间要共享的变量
	// 写线程B
	// atomvalue = 6


	//大家可以把原子操作理解为一种：不需要用到互斥量加锁（无锁）技术的多线程并发编程方式
	// 原子操作：在多线程中不会被打断的程序执行片段。 效率比互斥量更胜一筹
	// 互斥量的加锁一般是针对一个代码段（几行代码），原子操作针对的一般都是一个变量，而不是一个代码段
	
	//原子操作，一般都是指“不可分割的操作”，也就是说要么是完成的，要么是没完成的，不可能出现半完成
	//std::atomic来代表原子操作，是个类模版，其实std::atomic这个东西是用来封装某个类型的值的
	// (3.2) 基本的std::atomic用法案例
	// (3.3) 心得，一般用于计数或者统计（累计发送出去了多少个数据包，累计接收到了多少个数据包）

	/*std::thread mytobj1(mythread);
	std::thread mytobj2(mythread);
	mytobj1.join();
	mytobj2.join();
*/
	std::thread mytobj1(mythread);
	std::thread mytobj2(mythread);
	std::chrono::milliseconds dura(5000);
	std::this_thread::sleep_for(dura);
	g_ifend = true; //对原子对象的写操作，让线程自行结束
	mytobj1.join();
	mytobj2.join();

	return 0;
}




//int main()
//{
//	//一 std::future的其他成员函数
//	
//
//
//	std::cout << "main " << "thread_id:" << std::this_thread::get_id() << std::endl;
//	std::future<int> result = std::async(mythread); //绑定关系，流程不卡在这里
//	//std::future<int> result = std::async(std::launch::deferred, mythread);
//	std::cout << "continue... " << std::endl;
//	//std::cout << result.get() << std::endl; //卡在这里等待mythread执行完
//
//	//wait_for 等待一定时间
//	std::future_status status = result.wait_for(std::chrono::seconds(6)); //等待1秒
//
//	if (status == std::future_status::timeout) //超时：我想等待你1秒，希望你返回，你没有返回，那么status=timeout
//	{
//		//超时：表示线程还没执行完
//		std::cout << "超时，线程还没执行完" << std::endl;
//
//	}
//	else if (status == std::future_status::ready)
//	{
//		//线程返回成功
//		std::cout << "线程成功执行完毕，返回" << std::endl;
//		std::cout << result.get() << std::endl;
//	}
//	else if (status == std::future_status::deferred) //延迟
//	{
//		//如果async的第一个参数被设置为std::launch::deferred，则本条件成立
//		std::cout << "线程被延迟执行" << std::endl;
//		std::cout << result.get() << std::endl;
//	}
//
//	std::cout << " I Love China";
//
//	return 0;
//
//	//二 std::shared_future
//}
