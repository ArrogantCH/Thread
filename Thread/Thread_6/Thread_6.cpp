// Thread_6.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <mutex>


// 主线程创建单例
//class Singleton //这是一个单例类
//{
//private:
//	Singleton() {} //私有化了构造函数
//
//private:
//	static Singleton *singleton; //静态成员变量
//
//public:
//	static Singleton* getInstance()
//	{
//		if (singleton == NULL)
//		{
//			singleton = new Singleton();
//			static
//			static CGarRecycle cl; //当程序退出的时候，肯定会调用CGarRecycle的析构函数
//		}
//		return singleton;
//	}
//
//	class CGarRecycle  //类中套类，用来释放对象
//	{
//	public:
//		~CGarRecycle()
//		{
//			if (Singleton::singleton)
//			{
//				delete Singleton::singleton;
//				Singleton::singleton = NULL;
//			}
//		}
//
//	};
//
//	void fun()
//	{
//		std::cout << "测试" << std::endl;
//	}
//};

std::mutex resource_mutex;
std::once_flag g_flag; //这个是系统定义的标记
// 多个子线程创建单例
class Singleton //这是一个单例类
{
private:
	Singleton() {} //私有化了构造函数

private:
	static Singleton *singleton; //静态成员变量

public:
	//static Singleton* getInstance()
	//{
	//	//提高效率
	//	if (singleton == NULL) //双重锁定（检查）
	//	{
	//		std::unique_lock<std::mutex> mymutex(resource_mutex);
	//		if (singleton == NULL)
	//		{
	//			singleton = new Singleton();
	//			//static
	//			static CGarRecycle cl; //当程序退出的时候，肯定会调用CGarRecycle的析构函数
	//		}
	//	}
	//	return singleton;
	//}


	static void CreateInstance()  //只被调用一次
	{
		singleton = new Singleton();
		static CGarRecycle cl; //当程序退出的时候，肯定会调用CGarRecycle的析构函数
	}

	static Singleton* getInstance()
	{
		std::call_once(g_flag, CreateInstance); //两个线程同时执行到这里，其中一个线程需要等另外一个线程执行完毕CreateInstance
		return singleton;
	}

	class CGarRecycle  //类中套类，用来释放对象
	{
	public:
		~CGarRecycle()
		{
			if (Singleton::singleton)
			{
				delete Singleton::singleton;
				Singleton::singleton = NULL;
			}
		}

	};

	void fun()
	{
		std::cout << "测试" << std::endl;
	}
};


//静态变量初始化
Singleton *Singleton::singleton = NULL;
//这两种是不是一个意思？
//Singleton Singleton::*singleton = NULL;

void mythread()
{
	std::cout << "我的线程开始了" << std::endl;
	Singleton *singleton = Singleton::getInstance();
}

int main()
{
	// 一 设计模式大概谈
	// “设计模式”： 代码的一些写法（这些写法跟常规写法不怎么一样）：程序灵活，维护起来可能方便，但是别人接管，阅读代码很痛苦
	// 用“设计模式”理念写出来的代码是很晦涩的；《head first》
	//
	// 二 单例设计模式
	// 使用的频率比较高，
	// 单例：在整个项目中，有某个或者某些特殊的类，属于该类的对象，只能创建一个，多了创建不了

	Singleton *singleton = Singleton::getInstance(); //创建一个对象，返回该类（Singletond）对象的指针
	//Singleton *singleton2 = Singleton::getInstance();
	singleton->fun();  //该装载的数据装载了

	// 三 单例设计模式共享数据问题分析，解决
	// 在使用单例模式时，最好是在主线程中把单例类创建，并且初始化，这样在下面再有多个线程，数据也可以共享
	// 面临的问题：需要在我们自己创建的线程（而不是主线程）中来创建Singleton这个单例类的对象，这种线程可能不止一个（最少两个）
	// 我们可能会面临getInstance()这种成员函数需要互斥
	// 虽然这两个线程是同一个入口函数，但千万要记住，这是两个线程，同时执行getinstance()

	std::thread mytobj1(mythread);
	std::thread mytboj2(mythread);

	mytobj1.join();
	mytboj2.join();
	
	//四 std::call_once c++11引入的函数，该函数的第二个参数 是一个函数名a()
	//call_once功能是能够保证函数a()只被调用一次
	//call_once具备互斥量这种能力，而且效率上，比互斥量消耗的资源更少
	//call_once需要与一个标记结合使用，这个标记 std::once_flag， 其实once_flag是一个结构
	//call_once就是通过这个标记来决定对应的函数a()是否执行，调用call_once()成功后，call_once()就把这个标记设置为一种已调用状态
	//后续再次调用call_once()，只要once_flag被设置为了“已调用”状态，那么对应的函数a()就不会再被执行；
	

	return 0;
}

