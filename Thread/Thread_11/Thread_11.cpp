// Thread_11.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <mutex>

#include <Windows.h>

//#define _WINDOWSJQ_

//本类用于自动释放windows下的临界区，防止忘记LeaveCriticalSection导致死锁，
class CWinLock  //叫RAII类(Resource Acquisition is initialization) 资源获取及初始化
				//容器，智能指针等等
{
public:
	CWinLock(CRITICAL_SECTION *m_pCriticaltmp)  //构造函数
	{
		m_pCritical = m_pCriticaltmp;
		EnterCriticalSection(m_pCritical);
	}

	~CWinLock()  //析构函数
	{
		LeaveCriticalSection(m_pCritical);
	}

private:
	CRITICAL_SECTION *m_pCritical;

};

class A
{
public:
	//把收到的消息（玩家命令） 入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
#ifdef _WINDOWSJQ_
			//EnterCriticalSection(&my_winsec); //用临界区之前要先初始化
			//msgRecvQueue.push_back(i);
			//LeaveCriticalSection(&my_winsec);
			CWinLock wlock(&my_winsec);
			CWinLock wlock2(&my_winsec); //windows连续两次没问题
			msgRecvQueue.push_back(i);
#else
			//my_mutex.lock();
			//my_mutex.lock(); //报异常，和windows有区别
			//std::lock_guard<std::mutex> sbguard(my_mutex);
			std::chrono::microseconds timeout(100);
			//if (my_mutex.try_lock_for(timeout)) //等待100ms来尝试获取锁
			//{
			//	//在这100ms内拿到了锁
			//	msgRecvQueue.push_back(i);
			//	my_mutex.unlock(); //用完了要解锁
			//}
			//else
			//{
			//	//没拿到锁
			//	std::chrono::microseconds timeout2(100);
			//	std::this_thread::sleep_for(timeout2);
			//}

			if (my_mutex.try_lock_until(std::chrono::steady_clock::now() + timeout)) //等待100ms来尝试获取锁
			{
				//在这100ms内拿到了锁
				msgRecvQueue.push_back(i);
				my_mutex.unlock(); //用完了要解锁
			}
			else
			{
				//没拿到锁
				std::chrono::microseconds timeout2(100);
				std::this_thread::sleep_for(timeout2);
			}

			//msgRecvQueue.push_back(i);
			//my_mutex.unlock();
			//my_mutex.unlock();
#endif
		}
	}



	bool outMsgLULProc(int &command)
	{
#ifdef _WINDOWSJQ_
		EnterCriticalSection(&my_winsec); //用临界区之前要先初始化
		if (!msgRecvQueue.empty())
		{
			//消息不为空
			command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在 
			msgRecvQueue.pop_front(); //移除第一个元素，但不返回
			my_mutex.unlock();
			return true;
		}
		LeaveCriticalSection(&my_winsec);
#else
		my_mutex.lock();
		if (!msgRecvQueue.empty())
		{
			//消息不为空
			command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在 
			msgRecvQueue.pop_front(); //移除第一个元素，但不返回
			my_mutex.unlock();
			return true;
		}
		my_mutex.unlock();
#endif
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

	A()
	{
#ifdef _WINDOWSJQ_
		InitializeCriticalSection(&my_winsec); //用临界区之前要先初始化
#endif
	}

	//void testfun1()
	//{
	//	std::lock_guard<std::recursive_mutex> sbguard(my_mutex);
	//	//do something...
	//	testfun2();
	//}

	//void testfun2()
	//{
	//	std::lock_guard<std::recursive_mutex> sbguard(my_mutex);
	//	//do something...
	//}

private:
	std::list<int> msgRecvQueue; //容器，用于命令列表
	//std::mutex my_mutex; //创建了一个互斥量,一把锁
	//std::recursive_mutex my_mutex; //递归独占互斥量
	std::timed_mutex my_mutex; //是带超时功能的独占互斥量
#ifdef _WINDOWSJQ_
	CRITICAL_SECTION my_winsec; //windows中的临界区，非常类似于c++中的mutex
#endif

};

int main()
{
	//一  windows临界区
	//二	  多次进入临界区试验
	//		在同一个线程(不同线程就会卡住等待)中，windows中的“相同临界区变量”代表的临界区的进入(EnterCriticalSection)可以被多次调用
	//		但是调用了几次EnterCriticalSection，就得调用几次LeaveCriticalSection
	//      c++11 中的mutex不允许同一个线程中lock同一个互斥量多次，会报异常

	//三  自动析构技术
	//std::lock_guard

	//四 recuraive_mutex 递归的独占互斥量
	// std::mutex： 独占互斥量，自己lock时别人lock不了
	// recuraive_mutex： 递归的独占互斥量，允许同一个线程，同一个互斥量多次.lock()，效率上比mutex要差一些
	//	recuraive_mutex也有lock，也有unlock()
	//	考虑代码是否有优化空间

	//五 带超时的互斥量std::timed_mutex和std::recursive_timed_mutex
	// std::timed_mutex: 是带超时功能的独占互斥量
		// try_lock_for() : 参数是一段时间，等待一段时间。如果拿到了锁，或者等待超过时间没拿到锁，就走下来
		// try_lock_unti() : 参数是一个未来的时间点，在这个未来的时间没到的时间内，如果拿到了锁，那么就走下了；如果时间到了，没拿到锁，程序流程也走下来
	// std::recursive_timed_mutex：是带超时功能的递归独占互斥量(允许同一个线程多次获取这个互斥量)



	A myobja;
	std::thread myOutnMsgobj(&A::outMsgRecvQueue, &myobja); //第二个参数是 引用，才能保证线程理用的是同一个对象
	std::thread myInMsgobj(&A::inMsgRecvQueue, &myobja);

	myInMsgobj.join();
	myOutnMsgobj.join();

}
