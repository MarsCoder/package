/*
 * option.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: mars
 */
#include <sys/socket.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <map>
#include "option.h"
#include "common.h"

//extern std::map<int, std::string> etherIdMap;

//帧名字和帧ID
std::map<int, std::string> etherIdMap = { {ETH_P_IP, "ip"}/*,{ETH_P_IPV6, "ipv6"},
		{ETH_P_ARP, "arp"},{ETH_P_RARP, "rarp"},{ETH_P_LOOP, "loopback"}*/};

option::option() {
	// TODO Auto-generated constructor stub
	sockfd = -1;
	promiscF = 0;
	promiscFlags = 0;
	promiscFlags = ~promiscFlags;		//将混杂模式标记设为全1
	bufSize = RCV_BUF_SIZE_DEFAULT;		//设定默认缓冲区大小
	ifName = (char*)"erh0";					//设定默认网卡接口名字
	printMacFlags = false;				//默认不打印数据链路层头部信息
	etherProtocol = ETH_P_ALL;			//默认抓取所有
}

option::~option() {
	// TODO Auto-generated destructor stub
	if(promiscF){	//如果设定了混杂模式，则恢复到程序运行前状态
		setPromiscP(promiscFlags);
	}
}

////设定混杂模式
int option::setPromiscP(int iFlags)
{
	int iRet = -1;
	struct ifreq stIfr;

	/* 获取接口属性标志位 */
	strcpy(stIfr.ifr_name, ifName);
	iRet = ioctl(sockfd, SIOCGIFFLAGS, &stIfr);
	if (0 > iRet)
	{
		perror("[Error]Get Interface Flags");
		return -1;
	}
	if(promiscFlags == ~(0)){
		//记录程序运行前网卡是否处于混杂模式
		promiscFlags = (stIfr.ifr_flags || ~IFF_PROMISC) && IFF_PROMISC;
	}
	if (0 == iFlags)
	{
		/* 清除混杂模式标记位 */
		stIfr.ifr_flags &= ~IFF_PROMISC;
	}
	else
	{
		/* 设置为混杂模式标记位 */
		stIfr.ifr_flags |= IFF_PROMISC;
	}
	/* 设置接口属性标志位 */
	iRet = ioctl(sockfd, SIOCSIFFLAGS, &stIfr);
	if (0 > iRet)
	{
		perror("[Error]Set Interface Flags");
		return -1;
	}

	return 0;
}

////设置是否需要进入混杂模式的标记
void option::setPromiscF(bool f)
{
	promiscF = f;
}

//设置缓冲区大小
int option::setBufP()
{
	int i;
	/* 设置SOCKET选项 SO_RCVBUF设置缓冲区大小*/
    i = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufSize,sizeof(int));
    if (0 > i)
    {
        perror("[Error]Set socket option");
        //close(sockfd);
        return -1;
    }
    return 0;
}

//指定网络接口
void option::setInterface(char* name)
{
	ifName = name;
}

//绑定网络接口
int option::bindInterfaceP()
{
	int i;
	struct ifreq stIf;
	struct sockaddr_ll stLocal = {0};

	strcpy(stIf.ifr_name, ifName);
	i = ioctl(sockfd, SIOCGIFINDEX, &stIf);
	if (0 > i)
	{
		perror("[Error]Ioctl operation");
		return -1;
	}

	/* 绑定网络接口 */
	stLocal.sll_family = PF_PACKET;
	stLocal.sll_ifindex = stIf.ifr_ifindex;
	stLocal.sll_protocol = htons(etherProtocol);
	i = bind(sockfd, (struct sockaddr *)&stLocal, sizeof(stLocal));
	if (0 > i)
	{
		perror("[Error]Bind the interface");
		return -1;
	}
	return 0;
}

//创建套接字
int option::creatSocket(int family, int type)			//创建连接
{
	sockfd = socket(family, type, htons(etherProtocol));
	if(promiscF){	//如果设置了混杂模式标记，则设置混杂模式
		if(setPromiscP(true) == -1){
			close(sockfd);
			return -1;
		}
	}
	if(setBufP() == -1){	//设置缓冲区大小
		close(sockfd);
		return -1;
	}

	if(bindInterfaceP() == -1){	//绑定网卡接口
		close(sockfd);
		return -1;
	}
	return sockfd;
}

//返回缓冲区大小
int option::getBufSize()
{
	return bufSize;
}

//解释参数
int option::parseOpt(int argc, char **argv)
{
	int c;
	while((c = getopt(argc, argv, "i:b:p")) != -1){
		switch(c){
		case 'i':		//指定网络接口
			setInterface(optarg);
			break;
		case 'b':		//在数据链路层上选择网络协议（设定以太网贞类型）
			if(entherPotocol(optarg) == -1){
				return -1;
			}
			break;
		case 'p':		//设置是混杂模式
			setPromiscF(true);
			break;
		case 'e':		//在输出时打印数据链路层的信息
			setPrintMacFlags(true);
			break;
		case '?':
			std::cerr << "argument error!!!!!!!" << std::endl;
			return -1;
		}
	}
	return 0;
}

//指定数据链路层网络协议
int option::entherPotocol(char* protocol)
{
	int i = -1;
	if((i = pairEthNameId(protocol)) == -1){
		std::cerr << "unknow ethernet type" << std::endl;
		return -1;
	}

	switch(i){
	case ETH_P_IP:
		etherProtocol = ETH_P_IP;
		break;
	case ETH_P_IPV6:
		etherProtocol = ETH_P_IPV6;
		break;
	case ETH_P_ARP:
		etherProtocol = ETH_P_ARP;
		break;
	case ETH_P_RARP:
		etherProtocol = ETH_P_RARP;
		break;
	case ETH_P_LOOP:
		etherProtocol = ETH_P_LOOP;
		break;
	default:
		std::cerr << "unknow enthernet protocol \"" << protocol << " \" " << std::endl;
		return -1;

	}
	return 0;
}

//设置输出数据链路层信息标记
void option::setPrintMacFlags(bool flags)
{
	printMacFlags = flags;
}

std::string option::pairEthNameId(int id)	//给定帧id，返回帧名字
{
	std::map<int, std::string>::iterator i = etherIdMap.find(id);
	if (i == etherIdMap.end()){
		std::cerr << "unknow enthernet protocol \"" << " \" " << std::endl;
		return "unknow";
	}
	return (*i).second;	//返回帧名字
}


int	option::pairEthNameId(std::string protocol)	//给定帧名字，返回帧ID
{
	std::map<int, std::string>::iterator i = etherIdMap.begin();
	for(; i != etherIdMap.end(); ++i){
		if((*i).second == protocol){
			return (*i).first;
		}
	}
	return -1;
}
