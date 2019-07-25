// Thread_4.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <mutex>

class A
{
public:
	//把收到的消息（玩家命令） 入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
			std::lock_guard<std::mutex> sguard(my_mutex);
			//my_mutex.lock();
			msgRecvQueue.push_back(i);
			//my_mutex.unlock();
		}
	}

	

	bool outMsgLULProc(int &command)
	{
		std::lock_guard<std::mutex> sguard(my_mutex); //sguard是随便起的对象名
													// 针对作用域
													// lock_guard构造函数里执行了mutex::lock()
													// lock_guard析构函数里执行了mutex::unlock()
		//my_mutex.lock();
		if (!msgRecvQueue.empty())
		{
			//消息不为空
			command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在 
			msgRecvQueue.pop_front(); //移除第一个元素，但不返回
			//my_mutex.unlock();
			return true;
		}
		//my_mutex.unlock();
		return false;
	}

	//把收到的消息（玩家命令） 取出的线程
	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; i++)
		{
			bool result = outMsgLULProc(command);
			if (result)
			{
				std::cout << "outMsgRecvQueue执行，取出一个元素:" << command << std::endl;
				//可以考虑进行命令(数据)处理
			}
			else
			{
				std::cout << "outMsgRecvQueue执行，但目前消息队列被锁" << i << std::endl;
			}
		}
		std::cout << "end";
	}

private:
	std::list<int> msgRecvQueue; //容器，用于命令列表
	std::mutex my_mutex; //创建了一个互斥量,一把锁
	std::mutex my_mutex1; //创建了一个互斥量,一把锁
	std::mutex my_mutex2; //创建了一个互斥量,一把锁

};

int main()
{
	A myobja;
	std::thread myOutnMsgobj(&A::outMsgRecvQueue, &myobja); //第二个参数是 引用，才能保证线程理用的是同一个对象
	std::thread myInMsgobj(&A::inMsgRecvQueue, &myobja);

	myInMsgobj.join();
	myOutnMsgobj.join();
	
	//保护共享数据，操作时，某个线程用代码把共享数据锁住，其他想操作的线程必须等待解锁
	//一  互斥量(mutex)的基本概念
	// 互斥量是个类对象，理解成一把锁，多个线程尝试用lock()成员函数来加锁这把锁头，只有一个线程能锁定成功(成功的标志是lock()成功返回)
	//					如果没锁成功，那么流程卡在lock()这里，不断的尝试加这把锁
	// 互斥量使用要小心，保护数据不多也不少，少了，没达到保护效果，多了，影响效率

	//二 互斥量用法
	//(2.1) lock(),unlock()
	// 步骤：先lock()，操作共享数据，unlock()
	// lock()和unlock()要成对儿使用，有lock()必然有unlock(),每调用一次lock()，必然应该调用一次unlock();
	// 不应该也不允许调用1次lock()，却调用了两次unlock()；也不运行调用2次lock(),却调用了1次unlock()
	// 有lock，忘记unlock的问题，非常难排查
	// 为了防止大家忘记unlock()，引入了一个叫std::lock_guard的类模板：你忘了unlock不要紧，我替你unlock()
	// 学习过智能指针(unique_prt<>)：你忘记释放内存不要紧，我给你释放
	// （2.2） std::lock_guard类模版：直接取代lock(),unlock()，也就是说，用了lock_guard后，再不能使用lock()和unlock()
	
	// 三 死锁
	// 张三：站在北京 等李四
	// 李四：站在深圳 等张三
	// c++中：
	// 比如有两把锁（死锁这个问题，是由至少两个锁头也就是两个互斥量才能产生）：金锁（Jinlock），银锁（YinLock）
	// 两个线程A，B
	// （1）线程A执行的时候，这个线程先锁金锁，把金锁lock()成功了，然后它去尝试lock银锁
	//  出现了上下文切换
	//  (2)线程B执行了，这个线程先锁银锁，把银锁lock()成功了，然后它去尝试lock金锁
	//  此时此刻，死锁就产生了
	//  (3)线程A拿不到银锁头，流程走不下去（后面代码有解锁金锁头，但是流程走不下去，所以金锁头解不开）
	//  (4)线程B拿不到金锁头，流程走不下去（后面代码有解锁银锁头，但是流程走不下去，所以银锁头解不开）
	// (3.1) 死锁的一般解决方案：只要两个线程上锁的顺序一样就可以
	// (3.2) std::lock()函数模版：用来处理多个互斥量的时候才出场
	// 能力：一次锁住两个或者两个以上的互斥量(至少连个，多个不限，1个不行)
	// 它不存在这种因为在多个线程中，因为锁的顺序问题导致死锁的风险问题
	//  std::lock(): 要么两个互斥量都锁住，要么两个互斥量都没锁。（意思就是当锁住第一个后，另外一个被别的线程锁住了，它锁不住，则立即把已经锁定的解锁）
		/*void inMsgRecvQueue()
		{
			for (int i = 0; i < 100000; i++)
			{
				std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
				std::lock(my_mutex1, my_mutex2);
				msgRecvQueue.push_back(i);
				my_mutex2.unlock();
				my_mutex2.unlock();

			}
		}*/
	// (3.3) std::lock_guard的std::adopt_lock参数
	//  std::adopt_lock是个结构体对象，起一个标记作用：表示这个互斥量已经lock()，不需要在std::lock_guard<std::mutex>构造函数里对mutex lock()了
	//  总结：std::lock():一次锁定多个互斥量，谨慎使用(建议一个一个锁)
	/*void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
			std::lock(my_mutex1, my_mutex2);
			std::lock_guard<std::mutex> sguard1(my_mutex1, std::adopt_lock);
			std::lock_guard<std::mutex> sguard2(my_mutex2, std::adopt_lock);

			msgRecvQueue.push_back(i);
			my_mutex2.unlock();
			my_mutex2.unlock();

		}
	}*/
	return 0;
}

