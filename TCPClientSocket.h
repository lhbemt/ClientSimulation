#define _CRT_SECURE_NO_WARNINGS
#ifndef __CTCPCLIENTSOCKET__H
#define __CTCPCLIENTSOCKET__H
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include "NetMessage.h"
#include <vector>
#include "ThreadPoolTask.h"
#include <atomic>

class CTCPSocketManage;

struct PackageSend 
{
	char* pData;
	int   nLen;
	bool  bRet; // 消息是否被处理
};

struct PackageRecv
{
	char* pData;
	int   nLen;
	bool  bRet; // 是否是空闲的
};

enum class LoginMG // 登录M还是G
{
	LOGIN_M, // 登录M
	LOGIN_G, // 登录G
};

// 客户端socket

class CTCPClientSocket
{
public:
	CTCPClientSocket();
	~CTCPClientSocket();
	friend CTCPSocketManage;
public:
	bool Init();
	bool Connect(std::string strIPAddr, int nPort); // 连接
	int  SendData(void* pData, int nSize, int nMainID, int nAssistID); //发送数据
	void UDPRecvData(void* pBuff, int& nLen, struct PackageSend* pSend); // udp接受数据
	void HandleData(); // 处理接收的数据
	void CloseSocket(); // 关闭socket
public:
	const char* GetErrorMsg();
	void SetSecretKey(int nSecretKey); // 设置密钥
	int GetSecretKey();
	void SetEncrypt(bool bEncrypt); // 是否加密
	bool GetEncrypt();
	void SetReserve(int nReserve); // 保留字段
	int  GetReserve();
	void SetType(LoginMG mg);
	LoginMG GetType();
	void SetUserID(int nUserID);
	int  GetUserID();
	void SetExit(bool bExit);
private:
	void SendDataPackage(void* pData);
	void RecvDataPackage(void* pData);
	void UDPRecvDataPackage(void* pData);
	void RecvData(struct PackageRecv* pSend); // 接收数据
	void HandleData(void* pData, int nLen);

private:
	void HandleConnectMessage(void* pData, int nMainID, int nAssistID);

private:
	void CleanUpSocket();
	void SetError();
private:
	SOCKET m_sTCPSocket; // TCPSocket
	SOCKET m_sUDPSocket; // UDPSocket
	std::string m_strErrorMsg;

	int m_nSecretKey; // 密钥
	bool m_bEncrypt; // 是否加密
	bool m_bConnect; // 是否连接成功
	bool m_bExit; // 等待退出
	int  m_nReserve;
	LoginMG m_Type; // 是M还是G服务器
	int  m_nUserID; // 与之对应的userid

	std::string m_strIP;
	int m_nPort;

	int m_nRecvMaxNum; // 最多处理多少数据
	std::vector<struct PackageRecv*> m_vectPackageRecv; // 接收缓冲区
};

#endif