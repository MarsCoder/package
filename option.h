/*
 * option.h
 *
 *  Created on: Apr 13, 2015
 *      Author: mars
 */

#ifndef OPTION_H_
#define OPTION_H_

#include <string>

class option {
private:
	int		sockfd;
	int		bufSize;		//缓冲区大小
	bool	promiscF;		//用于标记是否需要进入混杂模式
	short	promiscFlags;	//用于记录运行程序前是否进入混杂模式
	char*	ifName;			//网卡接口名字
	short	etherProtocol;	//以太网帧类型
	bool	printMacFlags;	//输出数据链路层信息标记

	int setPromiscP(int iFlags);	//设定混杂模式
	int bindInterfaceP();			//绑定网卡
	int setBufP();					//设置缓冲区大小
	void setPrintMacFlags(bool);	//设置输出数据链路层信息标记
	int entherPotocol(char*);		//指定数据链路层网络协议
	void setInterface(char*);		//指定网卡接口
	void setPromiscF(bool);			//设置是否需要进入混杂模式的标记


public:
	option();
	virtual ~option();

	int creatSocket(int family, int type);	//创建套接字
	int parseOpt(int argc, char **argv);	//解释参数
	int getBufSize();			//返回缓冲区大小
	std::string pairEthNameId(int);	//给定帧id，返回帧名字
	int	pairEthNameId(std::string);	//给定帧名字，返回帧ID

};

#endif /* OPTION_H_ */
