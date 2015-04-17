/*
 * packet.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: mars
 */

#include <string>
#include <iostream>
#include <iomanip>
#include <deque>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>



#include "packet.h"
#include "MyBlockingQueue.h"

unsigned long packet::count = 0;
extern	MyBlockingQueue<char*> recvQue;	//缓冲区队列
extern std::map<int, std::string> etherIdMap;	//ip层协议类型id和对应的名字
//运输层协议类型id和对应的名字
std::map<unsigned short, std::string> ipIdNameMap = {
		{1, "ICMP"},{2, "IGMP"}, {4, "IP"}, {6, "TCP"},
		{8, "EGP"}, {9, "IGP"}, {17, "UDP"}, {41, "IPv6"},
		{50, "ESP"}, {89, "OSPF"}};

packet::packet() {
	// TODO Auto-generated constructor stub
	pPacket = NULL;
	etherHead = NULL;
	ipHead = NULL;
	tcpHead = NULL;
	udpHead = NULL;

}

packet::~packet() {
	// TODO Auto-generated destructor stub
}

//获得数据包指针
void packet::getpacket()
{
	pPacket = recvQue.pop();
	packet::count++;

}

//解析数据链路层首部
int packet::parseEthHead()
{
	etherHead = (struct ethhdr*) pPacket;

	    if (0 == etherHead)
	    {
	        return -1;
	    }

	    std::cout << "Eth-Pk-Type:"
	    		<< std::setw(5) << std::left << std::setfill(' ')<< std::hex
				<< ntohs(etherHead->h_proto)
				<<"("
	    		<< opt.pairEthNameId(ntohs(etherHead->h_proto))
				<< ") ";
	    if(opt.pairEthNameId(ntohs(etherHead->h_proto)) == "unknow"){
	    	return -1;
	    }
	    showMac(etherHead->h_source);	//源MAC地址
	    std::cout << ">> " ;
	    showMac(etherHead->h_dest);		//目的MAC地址
	    std::cout << std::endl;

	    return 0;
}

// 输出MAC地址
void packet::showMac(const unsigned char* macAddr)
{
    for(int i = 0; i < (ETH_ALEN - 1); ++i)
    {
        std::cout << std::setw(2)<< std::hex << std::right << std::setfill('0')
        	<< (unsigned short)macAddr[i] << ":";
    }
    std::cout << std::setw(2) << std::hex << (unsigned short)macAddr[ETH_ALEN-1];
}

//解析IP层数据包首部
int packet::parseIpv4Head()
{
	ipHead = (struct iphdr*)(pPacket + sizeof(struct ethhdr));
	if(ipHead == NULL){
		return -1;
	}
	std::cout << "IP-Pk-Type:"	//协议类型
			<< std::dec << std::setw(5) << std::setfill(' ') << std::left
			<< (unsigned short)ipHead->protocol			//协议类型id
			<< "("
			<< std::setw(5) << std::left << std::setfill(' ')
			<< pairIpNameId((unsigned short)ipHead->protocol)	//输出协议名称
			<<") ";
	std::cout<< inet_ntoa((struct in_addr&)ipHead->saddr);	//源地址
	std::cout<< " >> ";
	std::cout<< inet_ntoa((struct in_addr&)ipHead->daddr);	//目的地址
	std::cout << " IpHeadSize:"
			<< std::dec << (*ipHead).ihl * 4 ;	//协议头部长度
	std::cout << " IpPacketSize:"
				<< std::dec << std::setfill(' ') << std::right
				<< ntohs((*ipHead).tot_len) ;			//ip数据包数据段长度
	std::cout << " total recv " << packet::count <<" packet";
	std::cout << std::endl;

	return 0;
}

//解析TCP报文
int packet::parseTcpHead()
{
	//取出TCP头部的开始地址
	tcpHead = (struct tcphdr*)(pPacket + sizeof(struct ethhdr) + (ipHead->ihl * 4));
	if(tcpHead == NULL){
		return -1;
	}
	std::cout << "TCP-Pk-Type:"	//协议类型
			<< " SRT_PORT:"
			<< std::dec << std::setw(5) << std::setfill(' ') << std::left
			<< ntohs(tcpHead->source)	//源端口
			<< " DEST_PORT:"
			<< ntohs(tcpHead->dest)		//目的端口
			<< " SEQ:"
			<< ntohl(tcpHead->seq)		//序号
			<< " ACK_SEQ:"
			<< ntohl(tcpHead->ack_seq)	//确认号
			<< " DOFF:"
			<< ntohs(tcpHead->doff)	//数据偏移
			<< " FIN:"
			<< tcpHead->fin	//终止位
			<< " SYN:"
			<< tcpHead->syn	//同步位
			<< " RST:"
			<< tcpHead->rst	//复位位
			<< " PSH:"
			<< tcpHead->psh	//推送位
			<< " ACK:"
			<< tcpHead->ack	//确认位
			<< " URG:"
			<< tcpHead->urg	//紧急指针
			<< " window:"
			<< ntohs(tcpHead->window)	//窗口
			<< " check:"
			<< ntohs(tcpHead->check)	//校验和
			<< " urg_ptr:"
			<< ntohs(tcpHead->urg_ptr)	//紧急指针
			<< std::endl << std::endl;

	return 0;
}

//解析UDP报文首部
int packet::parseUdpHead()
{
	udpHead = (struct udphdr*)(pPacket + sizeof(struct ethhdr) + (ipHead->ihl * 4));
	if(udpHead == NULL){
		return -1;
	}
	std::cout << "UDP-Pk-Type:"	//协议类型
				<< " SRT_PORT:"
				<< std::dec << std::setw(5) << std::setfill(' ') << std::left
				<< ntohs(udpHead->source)	//源端口
				<< " DEST_PORT:"
				<< ntohs(udpHead->dest)		//目的端口
				<< " LEN:"
				<< ntohs(udpHead->len)
				<< " check:"
				<< ntohs(udpHead->check)
				<< std::endl << std::endl;

	return 0;
}

//取得 ip 协议名字
std::string packet::pairIpNameId(unsigned short id)
{
	std::map<unsigned short, std::string>::iterator i = ipIdNameMap.find(id);
	if(i != ipIdNameMap.end()){
		return (*i).second;
	}
	return "unknow";
}

//解析数据包
void packet::parsePacket()
{
	getpacket();	// 解析数据帧
	parseEthHead();	//解析数据链路层头部
	if(etherIdMap.find(ntohs(etherHead->h_proto)) != etherIdMap.end()){ //识别不出ip层协议，则不解析ip层
		//解析IP层首部部
		switch(ntohs(etherHead->h_proto)){
		case ETH_P_IP:
			parseIpv4Head();
			break;
		default:
			std::cout << "unknow IP layer protocol " << std::endl;
		}

		//解析运输层首部
		switch(ipHead->protocol){
		case 6:
			parseTcpHead();	//解析TCP报文首部
			break;
		case 17:
			parseUdpHead();//解析UDP报文首部
			break;
		default:
			std::cout << "unknow transport layer protocol " << std::endl;
			break;
		}
	}
	if(pPacket != NULL){
		free(pPacket);
	}
}
