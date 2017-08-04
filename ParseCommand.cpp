#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include "ParseCommand.h"
#include <stdarg.h>
#include <cctype>
#include <sstream>
using namespace std;


CParseCommand::CParseCommand()
{
	m_bQuit = false;
}


CParseCommand::~CParseCommand()
{
}

bool CParseCommand::ParseInput(const char* pInput)
{
	if (*pInput == 0)
	{
		PrintErrorMsg("invalid input");
		return false;
	}
	memset(m_szInput, 0, sizeof(m_szInput));
	m_nIndex = 0;
	strncpy(m_szInput, pInput, 512);

	FilterSpace(); // 先过滤空格
	int nIndex = m_nIndex;
	char ch;
	while (((ch = GetCh()) != ' ') && ch != 0)
	{
		if (!isalpha(ch))
		{
			PrintErrorMsg("invalid input: %s", m_szInput);
			return false;
		}
	}
	char szKey[20] = { 0x00 }; // 读取key关键字
	strncpy(szKey, m_szInput + nIndex, m_nIndex - nIndex);
	if (m_szInput[m_nIndex - 1] == ' ')
		szKey[m_nIndex - 1] = 0;
	return ParseCmd(szKey);
}

void CParseCommand::PrintErrorMsg(const char* p, ...)
{
	char szMsg[512] = { 0x00 };
	va_list arg;
	va_start(arg, p);
	vsprintf(szMsg, p, arg);
	m_strErrorMsg = std::move(std::string(szMsg));
}

bool CParseCommand::ParseCmd(const char* pCmd)
{
	int nToken;
	bool bHaveToken = TokensTable::FindToken(pCmd, nToken);
	if (!bHaveToken)
	{
		PrintErrorMsg("invalid keyword: %s", pCmd);
		return false;
	}

	switch (nToken)
	{
	case token_help:
		return ParseHelp();
	case token_set:
		return ParseSet();
	case token_loginM:
		return ParseLoginM();
	case token_loginG:
		return ParseLoginG();
	case token_quit:
		return ParseQuit();
	case token_sendM: // 发送数据 loopcount
		return ParseSendM();
	case token_sendG: // 发送数据 loopcount
		return ParseSendG();
	case token_reload:
		return ParseReload(); // 重新加载xml文件
	case token_loginMbatch:
		return ParseLoginMBatch(); // 批量登录M服务器
	case token_loginGbatch:
		return ParseLoginGBatch(); // 批量登录G服务器
	case token_sendbatch:
		return ParseSendBatch(); // 批量发送消息
	default:
		return true;
	}

}

bool CParseCommand::ParseHelp()
{
	std::cout <<"list of command: " << std::endl << std::endl;
	
	std::cout << "help:\t" << "输入help命令以查看支持的命令行" << std::endl;
	printf("\t%-10s", "example: help");
	printf("\n\n");

	std::cout << "set:\t" << "输入set命令以设置相关信息" << std::endl;
	printf("\t%-10s", "set secretkey\t设置密钥");
	printf("\n");
	printf("\t%-10s", "example: set secretkey 20170401");
	printf("\n");
	printf("\t%-10s", "set encrypt\t设置是否加密 0不加密 1加密");
	printf("\n");
	printf("\t%-10s", "example: set encrypt 1");
	printf("\n");
	printf("\t%-10s", "set connect\t设置连接参数");
	printf("\n");
	printf("\t%-10s", "example: set connect 192.168.1.106 3015\n");
	printf("\n");

	std::cout << "loginM:\t" << "用户登录M服务器\n"<< "\tloginM userid username password " << std::endl;
	printf("\t%-10s", "example: loginM 10027 hn7800 123456\n");
	printf("\n");

	std::cout << "loginMbatch: " << "批量用户登录M服务器\n" << "\tloginMbatch useridstart useridend prefixname prefixstart password " << std::endl;
	printf("\t%-10s", "example: loginMbatch 10018 10026 HN 1001 123456\n");
	printf("\n");

	std::cout << "loginGbatch: " << "批量用户登录G服务器\n" << "\tloginGbatch useridstart useridend prefixname prefixstart password nameid" << std::endl;
	printf("\t%-10s", "example: loginGbatch 10018 10026 HN 1001 123456 12100004\n");
	printf("\n");

	std::cout << "loginG:\t" << "用户登录G服务器\n" << "\tloginG userid username password nameid" << std::endl;
	printf("\t%-10s", "example: loginG 10027 hn7800 123456 12100004\n");
	printf("\n");

	std::cout << "sendM:\t" << "用户发送消息给M服务器\n" << "\tsendM mainid assistid loopcount " << std::endl;
	printf("\t%-10s", "example: sendM 136 2 1\n");
	printf("\n");

	std::cout << "sendG:\t" << "用户发送消息给G服务器\n" << "\tsendG mainid assistid loopcount " << std::endl;
	printf("\t%-10s", "example: sendG 181 2 1\n");
	printf("\n");

	std::cout << "sendbatch: " << "批量发送消息\n" << "\tsendbatch [M|G] mainid assistid loopcount" << std::endl;
	printf("\t%-10s", "example: sendbatch M 136 2 1000\n");
	printf("\n");

	std::cout << "reload:\t" << " 重新加载xml文件\n"<< "\t当修改了消息内容xml文件时，用此命令重新加载 " << std::endl;
	printf("\t%-10s", "example: reload");
	printf("\n\n");

	std::cout << "quit:\t" << " 退出该程序" << std::endl;
	printf("\t%-10s", "example: quit");
	printf("\n");
	return true;
}

bool CParseCommand::ParseSet()
{
	std::string szKeyWord = "";
	bool nRet = GetString(szKeyWord);
	if (!nRet)
		return false;
	int nToken;
	bool bHaveToken = TokensTable::FindToken(szKeyWord.c_str(), nToken);
	if (!bHaveToken)
	{
		PrintErrorMsg("invalid keyword: %s", szKeyWord.c_str());
		return false;
	}
	switch (nToken)
	{
	case token_secretkey: // 设置密钥
	{
		bool bRet;
		int nSecretKey = 0;
		bRet = GetNumber(nSecretKey);
		if (!bRet)
			return false;
		m_tcpSocketManage.SetSecretKey(nSecretKey);
		return true;
	}
	case token_encrypt: // 设置加密
	{
		bool bRet;
		int bEncrypt = 0;
		bRet = GetNumber(bEncrypt);
		if (!bRet)
			return false;
		m_tcpSocketManage.SetEncrypt((bEncrypt > 0) ? true : false);
		return true;
	}
	case token_connect: // 设置连接
	{
		std::string strIP = "";
		bool bRet;
		int nPort;
		bRet = GetIp(strIP);
		if (!bRet)
			return false;
		bRet = GetNumber(nPort);
		if (!bRet)
			return false;
		if (nPort <= 0)
		{
			std::cout << "invalid port: " << nPort << std::endl;
			return false;
		}
		m_tcpSocketManage.SetConnectParam(strIP, nPort);
		return true;
	}
	default:
		break;
	}
	return true;
}

bool CParseCommand::ParseEncrypt()
{
	return true;
}

bool CParseCommand::ParseLoginM()
{
	bool bRet;
	int nUserID;
	std::string strUserName;
	std::string strPassWord;
	bRet = GetNumber(nUserID);
	if (!bRet)
		return false;
	bRet = GetUserName(strUserName);
	if (!bRet)
		return false;
	bRet = GetPassWord(strPassWord);
	if (!bRet)
		return false;
	m_tcpSocketManage.UserLogin(nUserID, strUserName.c_str(), strPassWord.c_str(), LoginMG::LOGIN_M);
	m_bStart = true;
	return true;
}

bool CParseCommand::ParseLoginG()
{
	bool bRet;
	int nUserID;
	int nNameID;
	std::string strUserName;
	std::string strPassWord;
	bRet = GetNumber(nUserID);
	if (!bRet)
		return false;
	bRet = GetUserName(strUserName);
	if (!bRet)
		return false;
	bRet = GetPassWord(strPassWord);
	if (!bRet)
		return false;
	bRet = GetNumber(nNameID);
	if (!bRet)
		return false;
	if (nNameID <= 0)
	{
		std::cout << "invalid nameid " << nNameID << std::endl;
		return false;
	}
	m_tcpSocketManage.UserLogin(nUserID, strUserName.c_str(), strPassWord.c_str(), LoginMG::LOGIN_G, nNameID);
	m_bStart = true;
	return true;
}

bool CParseCommand::ParseQuit()
{
	std::cout << "waiting for quit." << std::endl;
	if (m_bStart)
		m_tcpSocketManage.Quit();
	else
		m_tcpSocketManage.SetStart(true); // 未登录时的退出
	m_bQuit = true;
	m_bStart = false;
	return true;
}

bool CParseCommand::ParseSendM()
{
	int nUserID = 0;
	bool bRet = GetNumber(nUserID);
	if (!bRet)
		return false;
	int nMainID = 0; // 主ID
	bRet = GetNumber(nMainID);
	if (!bRet)
		return false;
	int nAssistID = 0; // 辅助ID
	bRet = GetNumber(nAssistID);
	if (!bRet)
		return false;
	int nLoopCount = 0;
	bRet = GetNumber(nLoopCount); // 发送次数
	if (!bRet)
		return false;
	if (nLoopCount == 0)
		nLoopCount = 1;
	m_tcpSocketManage.SendData(nUserID, nMainID, nAssistID, nLoopCount, LoginMG::LOGIN_M);
	return true;
}

bool CParseCommand::ParseSendG()
{
	int nUserID = 0;
	bool bRet = GetNumber(nUserID);
	if (!bRet)
		return false;
	int nMainID = 0; // 主ID
	bRet = GetNumber(nMainID);
	if (!bRet)
		return false;
	int nAssistID = 0; // 辅助ID
	bRet = GetNumber(nAssistID);
	if (!bRet)
		return false;
	int nLoopCount = 0;
	bRet = GetNumber(nLoopCount); // 发送次数
	if (!bRet)
		return false;
	if (nLoopCount == 0)
		nLoopCount = 1;
	m_tcpSocketManage.SendData(nUserID, nMainID, nAssistID, nLoopCount, LoginMG::LOGIN_G);
	return true;
}

bool CParseCommand::ParseReload()
{
	bool bRet = false;
	std::string M_G; // 重新加载M或G的xml文件
	bRet = GetString(M_G);
	if (!bRet)
		return false;
	if (M_G == "M")
		m_tcpSocketManage.Reload(LoginMG::LOGIN_M);
	else if (M_G == "G")
		m_tcpSocketManage.Reload(LoginMG::LOGIN_G);
	else
		std::cout << "invalid input: " << M_G.c_str() << std::endl;
	return true;
}

bool CParseCommand::ParseLoginMBatch() // 批量登录M服务器
{
	bool bRet = false;
	int nUserStart; //用户IDstart
	int nUserEnd;   // 用户IDEnd
	std::string strPrefix; // 前缀
	int nStart; // 开始
	std::string strPassWord;
	bRet = GetNumber(nUserStart); // 开始
	if (!bRet)
		return false;
	bRet = GetNumber(nUserEnd); // 结束
	if (!bRet)
		return false;
	bRet = GetString(strPrefix); // 前缀
	if (!bRet)
		return false;
	bRet = GetNumber(nStart); // 开始的
	if (!bRet)
		return false;
	bRet = GetPassWord(strPassWord);
	if (!bRet)
		return false;
	for (int i = 0; i < (nUserEnd - nUserStart + 1); i++)
	{
		std::stringstream strStream;
		strStream << strPrefix;
		strStream << (nStart + i);
		std::string strName;
		strStream >> strName;
		m_tcpSocketManage.UserLogin(nUserStart + i, strName.c_str(), strPassWord.c_str(), LoginMG::LOGIN_M);
	}
	m_bStart = true;
	return true;
}

bool CParseCommand::ParseLoginGBatch()
{
	bool bRet = false;
	int nUserStart; //用户IDstart
	int nUserEnd;   // 用户IDEnd
	std::string strPrefix; // 前缀
	int nStart; // 开始
	int nNameID; //nameid
	std::string strPassWord;
	bRet = GetNumber(nUserStart); // 开始
	if (!bRet)
		return false;
	bRet = GetNumber(nUserEnd); // 结束
	if (!bRet)
		return false;
	bRet = GetString(strPrefix); // 前缀
	if (!bRet)
		return false;
	bRet = GetNumber(nStart); // 开始的
	if (!bRet)
		return false;
	bRet = GetPassWord(strPassWord);
	if (!bRet)
		return false;
	bRet = GetNumber(nNameID);
	if (!bRet)
		return false;
	if (nNameID <= 0)
	{
		std::cout << "invalid nameid " << nNameID << std::endl;
		return false;
	}
	for (int i = 0; i < (nUserEnd - nUserStart); i++)
	{
		std::stringstream strStream;
		strStream << strPrefix;
		strStream << (nStart + i);
		std::string strName;
		strStream >> strName;
		m_tcpSocketManage.UserLogin(nUserStart + i, strName.c_str(), strPassWord.c_str(), LoginMG::LOGIN_M, nNameID);
	}
	m_bStart = true;
	return true;
}

bool CParseCommand::ParseSendBatch() // 批量发送消息
{
	bool bRet;
	int nMainID;
	int nAssistID;
	int nLoopCount;
	std::string m_g;
	bRet = GetString(m_g);
	if (!bRet)
		return false;
	if (m_g == "M" || m_g == "G")
	{
		bRet = GetNumber(nMainID);
		if (!bRet)
			return false;
		bRet = GetNumber(nAssistID);
		if (!bRet)
			return false;
		bRet = GetNumber(nLoopCount);
		if (!bRet)
			return false;
		if (nLoopCount == 0)
			nLoopCount = 1;
		if (m_g == "M")
			m_tcpSocketManage.SendDataBatch(nMainID, nAssistID, nLoopCount, LoginMG::LOGIN_M);
		else
			m_tcpSocketManage.SendDataBatch(nMainID, nAssistID, nLoopCount, LoginMG::LOGIN_G);
		return true;
	}
	std::cout << "invalid input: " << m_g << std::endl;
	return false;
}

bool CParseCommand::GetString(std::string& str)
{
	FilterSpace(); // 过滤空格
	int nIndex = m_nIndex;
	char ch;
	while (((ch = GetCh()) != ' ') && ch != 0)
	{
		if (!isalpha(ch))
		{
			PrintErrorMsg("invalid input: %s", m_szInput);
			return false;
		}
	}
	char szParseString[512] = { 0x00 };
	strncpy(szParseString, m_szInput + nIndex, m_nIndex - nIndex);
	if (m_szInput[m_nIndex - 1] == ' ')
		szParseString[m_nIndex - nIndex - 1] = 0;
	str = szParseString;
	return true;
}

bool CParseCommand::GetNumber(int& nNum)
{
	FilterSpace(); // 过滤空格
	int nIndex = m_nIndex;
	char ch;
	while (((ch = GetCh()) != ' ') && ch != 0)
	{
		if (!isdigit(ch))
		{
			PrintErrorMsg("invalid input: %s, not number", m_szInput);
			return false;
		}
	}
	char szParseString[512] = { 0x00 };
	strncpy(szParseString, m_szInput + nIndex, m_nIndex - nIndex);
	if (m_szInput[m_nIndex - 1] == ' ')
		szParseString[m_nIndex - nIndex - 1] = 0;
	nNum = atoi(szParseString);
	return true;
}

bool CParseCommand::GetIp(std::string& strIP)
{
	FilterSpace(); // 过滤空格
	int nIndex = m_nIndex;
	char ch;
	while (((ch = GetCh()) != ' ') && ch != 0)
	{
		if (!isalpha(ch) && !isdigit(ch) && ch != '.')
		{
			PrintErrorMsg("invalid input: %s", m_szInput);
			return false;
		}
	}
	char szParseString[512] = { 0x00 };
	strncpy(szParseString, m_szInput + nIndex, m_nIndex - nIndex);
	if (m_szInput[m_nIndex - 1] == ' ')
		szParseString[m_nIndex - nIndex - 1] = 0;
	strIP = szParseString;
	return true;
}

bool CParseCommand::GetUserName(std::string& userName)
{
	FilterSpace(); // 过滤空格
	int nIndex = m_nIndex;
	char ch;
	while (((ch = GetCh()) != ' ') && ch != 0)
	{
		if (!isalpha(ch) && !isdigit(ch))
		{
			PrintErrorMsg("invalid input: %s", m_szInput);
			return false;
		}
	}
	char szParseString[512] = { 0x00 };
	strncpy(szParseString, m_szInput + nIndex, m_nIndex - nIndex);
	if (m_szInput[m_nIndex - 1] == ' ')
		szParseString[m_nIndex - nIndex - 1] = 0;
	userName = szParseString;
	return true;
}

bool CParseCommand::GetPassWord(std::string& passWord)
{
	return GetUserName(passWord);
}