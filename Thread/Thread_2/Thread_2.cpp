// Thread_2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <share.h>


class A
{
public:
	int m_i;
	//类型转换构造函数，可以把一个int转换成类A对象
	A(int a) : m_i(a) 
	{
		std::cout << "A(int a) : m_i(a)  构造函数执行" << this << " thread_id = "<< std::this_thread::get_id() <<std::endl;
	}
	A(const A &a) : m_i(a.m_i)
	{
		std::cout << "A(const A &a) : m_i(a.m_i)  拷贝构造函数执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
	}
	~A()
	{
		std::cout << "~A()  析构函数执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
	}

	void thread_work(int num)
	{
		std::cout << "子线程thread_work执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
	}

	void operator()(int num)
	{
		std::cout << "子线程operator执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
	}

};

//class A
//{
//public:
//	mutable int m_i; //mutable 无论怎样都可以修改这个值，包括const
//	//类型转换构造函数，可以把一个int转换成类A对象
//	A(int a) : m_i(a)
//	{
//		std::cout << "A(int a) : m_i(a)  构造函数执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
//	}
//	A(const A &a) : m_i(a.m_i)
//	{
//		std::cout << "A(const A &a) : m_i(a.m_i)  拷贝构造函数执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
//	}
//	~A()
//	{
//		std::cout << "~A()  析构函数执行" << this << " thread_id = " << std::this_thread::get_id() << std::endl;
//	}
//
//};



//void myprint(const int &i, char *pmybuf) (1)
//void myprint(const int i, char *pmybuf)  (2)
//void myprint(const int &i, const std::string &pmybuf) //(3)
//{
//	std::cout << i << std::endl; //分析任务，i并不是mvar的引用，所以，即便主线程detach了子线程，那么子线程中用i值仍然是安全的
//	std::cout << pmybuf.c_str() << std::endl;  //指针在detach子线程时，绝对有问题！
//	//std::cout << *pmybuf << std::endl;
//	return;
//}

//void myprint(const int &i, const A &pmybuf)
//{
//	std::cout << &pmybuf << std::endl;
//}


//void myprint2(const A &pmybuf)
//{
//	pmybuf.m_i = 199; //我们修改值是不会影响到main函数
//	std::cout<<"子线程myprint2的参数地址是"<< &pmybuf<< " thread_id = " << std::this_thread::get_id() << std::endl;
//}

//void myprint2(A &pmybuf)
//{
//	pmybuf.m_i = 199; //我们修改值是会影响到main函数的，因为用了std::ref
//	std::cout << "子线程myprint2的参数地址是" << &pmybuf << " thread_id = " << std::this_thread::get_id() << std::endl;
//}

//如果传只能指针
void myprint2(std::unique_ptr<int> pmybuf)
{
	//pmybuf.m_i = 199; //我们修改值是会影响到main函数的，因为用了std::ref
	std::cout << "子线程myprint2的参数地址是" << &pmybuf << " thread_id = " << std::this_thread::get_id() << std::endl;
}

int main()
{
    //一：传递临时对象作为线程参数
	//(1.1) 要避免的陷阱 （解释1）
	//(1.1) 要避免的陷阱 （解释2）
	// 事实1：只要用临时构造的A类对象作为参数传递给线程，那么就一定能够在主线程执行完毕前把现场函数的第二个参数构建出来，
	//			从而确保即便detach()子线程了也安全运行
	// (1.3)总结
	//  a.若传递int这种简单类型参数，建议都是值传递，不要用引用
	//  b.如果传递类对象，避免隐式类型转换，全部都在创建线程这一行就构建出临时对象来，然后在函数参数里用引用来接，
	//    否则系统还会构造一次，总共三次对象
	// 终极结论：
	//  c.建议不使用detach(),只使用join()，这样就不存在线程失效导致线程对内存的非法引用问题
	//int mvar = 1;
	//int &mvary = mvar;
	//char mybuf[] = "this is a test";
	////std::thread mytobj(myprint,mvary, mybuf); //如果使用(3)， 这里其实存在一个隐身转换，转换成string，
	//											//事实上存在，mybuf都被回收了（main函数执行完了）,系统才用mybuf去转string
	//std::thread mytobj(myprint, mvary, std::string(mybuf)); //我们这里直接将mybuf转换成string对象（生成一个临时对象），这是一个可以保证在线程中使用的方法
	
	//二、临时对象作为线程参数继续讲，常用测试大法
	//(2.1) 线程id概念： id是个数字，每个线程（不管是主线程还是子线程），每个线程实际上都对应一个数字，而且每个线程对应的这个id也不同
	// 也就是说，不同的线程，它的线程id(数字)必然是不同的
	// 线程id可以用c++标准库里面的函数来获取，std::this_thread::get_id()来获取
	// (2.2)临时对象构造时机抓捕
	//
	//三：传递类对象、智能指针作为线程参数
	//std::ref 函数；

	//四：用成员函数指针做线程函数 operator()








	//int mvar = 1;
	//int mysecondpar = 12;
	////std::thread mytobj(myprint, mvar, mysecondpar); //我们希望mysecondpar转成A类型对象传递给myprint的第二个参数
	//std::thread mytobj(myprint, mvar, A(mysecondpar));
	////mytobj.join();

	//std::cout << "主线程id：" << std::this_thread::get_id() << std::endl;
	//int mvar = 1;
	////std::thread mytobj(myprint2, mvar); //A类在子线程中构造
	//std::thread mytobj(myprint2, A(mvar)); //A类在主线程中构造


	//A myobj(10); //生成一个类对象
	//std::thread mytobj(myprint2, myobj); //将类对象作为参数
	//std::thread mytobj(myprint2, std::ref(myobj)); //将类对象作为参数,这里传递的是真引用

	/*std::unique_ptr<int> myp(new int(100));
	std::thread mytobj(myprint2, std::move(myp));
	mytobj.join();*/

	//mytobj.detach(); //子线程和主线程分别执行



	A myobj(10);
	//std::thread mytobj(&A::thread_work, myobj, 15);
	//std::thread mytobj(&A::thread_work, std::ref(myobj), 15);
	//std::thread mytobj(&A::thread_work, &myobj, 15); // &myobj == std::ref(myobj)

	std::thread mytobj(std::ref(myobj), 15); //不调用拷贝构造函数了，那么后续如果调用mytobj.detach()就不安全了

	mytobj.join();

	return 0;
}

