/*
 * MyBlockingQueue.h
 *
 *  Created on: Apr 13, 2015
 *      Author: mars
 */

#ifndef MYBLOCKINGQUEUE_H_
#define MYBLOCKINGQUEUE_H_

#include <deque>
#include <boost/thread.hpp>

template<class T>
class MyBlockingQueue {
private:
	boost::mutex mu;	//互斥量
	boost::condition_variable_any empty;  //条件变量
	std::deque<T> queBuf;	//缓冲区队列

public:
	MyBlockingQueue(){}
	~MyBlockingQueue(){}

	void push(const T x){	//添加元素
		boost::mutex::scoped_lock lock(mu);
		queBuf.push_back(x);
		empty.notify_one();	//通知等待数据的线程
	}
	//
	T pop( ){		//取出元素元素
		boost::mutex::scoped_lock lock(mu);
		while(queBuf.size() == 0){
			empty.wait(mu);	//等待条件变量成立
		}
		T back = queBuf.front();
		queBuf.pop_front();
		return back;
	}

};


#endif /* MYBLOCKINGQUEUE_H_ */
