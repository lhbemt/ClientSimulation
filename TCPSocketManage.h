#define _CRT_SECURE_NO_WARNINGS
#ifndef __CTCPSOCKETMANAGE__H
#define __CTCPSOCKETMANEGE__H
#include "TCPClientSocket.h"
#include "BalanceTree.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "ParseStruct.h"
#include "Log.h"

// TCP管理类
// 一个userid一个socket

struct ClientUserInfo // 浅拷贝就行 共用一个CTCPClientSocket
{
	int nUserID; // 用户ID
	char szUserName[20]; // 用户名
	CTCPClientSocket* socketHandle; // 该用户ID对应的socket

	ClientUserInfo()
	{
		nUserID = -1;
		socketHandle = nullptr;
	}

	bool operator ==(ClientUserInfo& rhs)
	{
		return nUserID == rhs.nUserID;
	}

	bool operator <(ClientUserInfo& rhs)
	{
		return nUserID < rhs.nUserID;
	}

	bool operator >(ClientUserInfo& rhs)
	{
		return nUserID > rhs.nUserID;
	}
};

class CTCPSocketManage
{
public:
	CTCPSocketManage();
	~CTCPSocketManage();

public:
	bool UserLogin(int nUserID, const char *szUserName, const char* szPassWord, LoginMG login, int nNameID = 0); // 用户登录
	bool UserLogOut(int nUserID); // 用户退出
	void SetSecretKey(int nSecretKey); // 设置密钥
	void SetEncrypt(bool bEncrypt); // 是否加密
	void SetConnectParam(std::string strIP, int nPort); //设置连接的服务器和密码
	bool UserSendMessage(int nUserID, int nMainID, int nAssistID); // 用户发送消息
	void SendData(int nUserID, int nMainID, int nAssistID, int nLoopCount, LoginMG login); // 发送消息 nLoopCount发送多少次
	void Reload(LoginMG login); // 重新加载xml文件
	void SendDataBatch(int nMainID, int nAssistID, int nLoopCount, LoginMG login);
	void SetStart(bool bStart);
	void Quit(); // 退出
private:
	void HandleData(ClientUserInfo user);
	void UserLogin(ClientUserInfo* userInfo, const char* szUserName, const char* szPassWord); // 登录M服务器
	void UserLogin(ClientUserInfo* userInfo, const char* szPassWord, int nNameID); // 登录G服务器
	void CloseAll(); // 关闭所有socket
private:
	CBalanceTree<struct ClientUserInfo> m_treeClientInfo;
	std::mutex       m_mutxUsers;

	int m_nSecretKey;
	bool m_bEncrypt;
	std::string m_strIP;
	int m_nPort;

	bool m_bExit; // 结束标志
	std::thread m_threadHandleData; // 处理所有数据的线程
	bool m_bStart; // 是否开始处理数据

	// 解析xml文件
	ParseStruct     m_mXmlMParse; // M服务器消息ID
	ParseStruct     m_mXmlGParse; // G服务器消息ID
};

#endif