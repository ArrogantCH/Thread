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
	std::cout << "主线程4" << std::endl;
	std::cout << "主线程5" << std::endl;
}

