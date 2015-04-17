/*
 * main.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: mars
 */

#include "common.h"
#include "option.h"
#include "packet.h"

#include <iostream>
#include <iomanip>
#include <deque>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

#include <boost/thread.hpp>

#include "MyBlockingQueue.h"

char recvBuf[RCV_BUF_SIZE] = {0};
MyBlockingQueue<char*> recvQue;	//缓冲区队列

void parseThread()
{
	packet*	pPacket;
	for(;;){
		pPacket = new packet;
		if(pPacket != NULL)
			pPacket->parsePacket();
		if(pPacket != NULL)
			delete pPacket;
	}
}

int main(int argc, char **argv)
{
	int fd = -1;
	int i = -1;
	option sock;
	socklen_t stFromLen = 0;
	if(sock.parseOpt(argc, argv) == -1){	//设置参数；
		return -1;
	}

	fd = sock.creatSocket(PF_PACKET, SOCK_RAW);	//创建套接字
    if(0 > fd)
    {	perror("socket error!");
        return -1;
    }

	boost::thread parseT(parseThread);
    while(1)
	{
		memset(recvBuf, 0, sock.getBufSize());	//清空接收缓冲区
		/* 接收数据帧 */
		i = recvfrom(fd, recvBuf, sock.getBufSize(), 0, NULL, &stFromLen);
		if (0 > i)
		{
			continue;
		}
		char* buf = (char*)malloc(i + 1);
		memcpy(buf, recvBuf, i +1);	//分配内存
		recvQue.push(buf);	//将数据放放进缓冲队列
	}
    parseT.join();

    return 0;

}


