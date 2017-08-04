#ifndef __CPARSECOMMAND__H
#define __CPARSECOMMAND__H
#include "Tokens.h"
#include <iostream>
#include "TCPSocketManage.h"

// 解析命令模块

class CParseCommand
{
public:
	CParseCommand();
	~CParseCommand();
public:
	bool ParseInput(const char* pInput);
	const char* GetErrorMsg()
	{
		return m_strErrorMsg.c_str();
	}

private:
	bool ParseCmd(const char* pCmd);
private:
	void PrintErrorMsg(const char* p, ...);
private:
	char GetCh()
	{
		return m_szInput[m_nIndex++];
	}
	void UnGetCh()
	{
		m_nIndex--;
	}
	bool GetString(std::string& str);
	void FilterSpace() // 过滤空格和tab
	{
		char ch;
		while ((ch = GetCh()) == ' ' || ch == '\t')
		{
			;
		}
		UnGetCh();
	}
	bool GetNumber(int& nNum); // 获得数字
	bool GetIp(std::string& strIP);
	bool GetUserName(std::string& userName);
	bool GetPassWord(std::string& passWord);
private:
// 解析命令
	bool ParseHelp();
	bool ParseSet();
	bool ParseEncrypt();
	bool ParseLoginM();
	bool ParseLoginG();
	bool ParseQuit();
	bool ParseSendM();
	bool ParseSendG();
	bool ParseReload();
	bool ParseLoginMBatch();
	bool ParseLoginGBatch();
	bool ParseSendBatch();
public:
	bool m_bQuit; // 退出
	bool m_bStart; // 是否连接了服务器
private:
	std::string m_strErrorMsg;
	char m_szInput[512];
	int  m_nIndex; // 解析到第几个字符
private:
	CTCPSocketManage m_tcpSocketManage; // socket管理模块
};


#endif