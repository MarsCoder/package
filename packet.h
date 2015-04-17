/*
* packet.h
*
*  Created on: Apr 13, 2015
*      Author: mars
*/

#ifndef packet_H_
#define packet_H_

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <map>
#include <string>

#include "option.h"
#include "common.h"

class packet {

private:
	char* 	pPacket;		//数据包指针
	option	opt;			//

	struct ethhdr	*etherHead;		//数据链路层首部
	struct iphdr	*ipHead;		//IP层首部
	struct tcphdr	*tcpHead;		//tcp首部
	struct udphdr	*udpHead;		//udp首部

public:
	static unsigned long count;

public:
	packet();
	virtual ~packet();
	void getpacket();	//获得整个数据包(地址)
	int parseEthHead();		//解析数据链路层数据包首部
	int parseIpv4Head();		//解析IP层数据包首部
	int parseTcpHead();		//解析TCP报文首部
	int	parseUdpHead();		//解析UDP报文首部
	void showMac(const unsigned char* macAddr);	//输出MAC地址
	std::string pairIpNameId(unsigned short);	//取得 ip 协议名字
	void parsePacket();	//解析数据包

};

#endif /* packet_H_ */
