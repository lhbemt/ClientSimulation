#include "TCPSocketManage.h"
#include "md5.h"


CTCPSocketManage::CTCPSocketManage() : m_mXmlMParse(".\\MStructDefine.xml"), m_mXmlGParse(".\\GStructDefine.xml")
{
	m_bExit = false;
	m_bStart = false;
	CLog::GetInstance()->Init();
	m_threadHandleData = std::thread([this]() {
		while (!m_bExit)
		{
			m_mutxUsers.lock();
			if (m_treeClientInfo.Empty() && m_bStart)
			{
				m_mutxUsers.unlock();
				break;
			}
			m_treeClientInfo.DoSomthing(std::bind(&CTCPSocketManage::HandleData, this, std::placeholders::_1));
			// 检测未连接的socket 并进行关闭

			m_mutxUsers.unlock();
		}
	}); // 处理数据
}


CTCPSocketManage::~CTCPSocketManage()
{
}

bool CTCPSocketManage::UserLogin(int nUserID, const char* szUserName, const char* szPassWord, LoginMG login, int nNameID)
{
	ClientUserInfo userLogin;
	userLogin.nUserID = nUserID;
	strcpy(userLogin.szUserName, szUserName);
	userLogin.socketHandle = new CTCPClientSocket;
	userLogin.socketHandle->SetSecretKey(m_nSecretKey);
	userLogin.socketHandle->SetEncrypt(m_bEncrypt);
	userLogin.socketHandle->SetUserID(nUserID);
	bool nRet;
	nRet = userLogin.socketHandle->Init();
	if (!nRet)
	{
		delete userLogin.socketHandle;
		return true;
	}
	nRet = userLogin.socketHandle->Connect(m_strIP, m_nPort); // 连接成功
	if (!nRet)
	{
		delete userLogin.socketHandle;
		return true;
	}
	else
	{
		m_mutxUsers.lock();
		m_treeClientInfo.InsertElement(userLogin);
		m_mutxUsers.unlock();
	}
	// 用户登录
	while (userLogin.socketHandle->GetReserve() == 0)
		std::this_thread::sleep_for(std::chrono::microseconds(10)); //等待赋值
	userLogin.socketHandle->SetType(login);
	if (login == LoginMG::LOGIN_M)
		UserLogin(&userLogin, szUserName, szPassWord); // 登录大厅
	else
		UserLogin(&userLogin, szPassWord, nNameID); // 登录G服务器
	m_bStart = true;
	return true;
}

bool CTCPSocketManage::UserLogOut(int nUserID)
{
	return false;
}

bool CTCPSocketManage::UserSendMessage(int nUserID, int nMainID, int nAssistID)
{
	return false;
}

void CTCPSocketManage::SetSecretKey(int nSecretKey) // 所有连接设置密钥
{
	m_nSecretKey = nSecretKey;
}

void CTCPSocketManage::SetEncrypt(bool bEncrypt) // 所有连接是否加密
{
	m_bEncrypt = bEncrypt;
}

void CTCPSocketManage::SetConnectParam(std::string strIP, int nPort)
{
	m_strIP = strIP;
	m_nPort = nPort;
}

void CTCPSocketManage::HandleData(ClientUserInfo user)
{
	user.socketHandle->HandleData();
}

void CTCPSocketManage::UserLogin(ClientUserInfo* userInfo, const char* szUserName, const char* szPassWord)
{
	MSG_GP_S_LogonByNameStruct loginPlace;
	memset(&loginPlace, 0, sizeof(loginPlace));

	loginPlace.bForced = false;
	loginPlace.iUserID = userInfo->nUserID;
	loginPlace.gsqPs = 5471;
	strcpy(loginPlace.szName, szUserName);

	MD5_CTX Md5;
	Md5.MD5Update((unsigned char *)szPassWord, lstrlen(szPassWord));
	unsigned char szMDTemp[16];
	Md5.MD5Final(szMDTemp);
	char szMD5Input[64];
	for (int i = 0; i < 16; i++)
	{
		wsprintf(&szMD5Input[i * 2], "%02x", szMDTemp[i]);
	}
	strcpy(loginPlace.szMD5Pass, szMD5Input);

	userInfo->socketHandle->SendData(&loginPlace, sizeof(loginPlace), MDM_GP_LOGON, ASS_GP_LOGON_BY_NAME); // 发送登录
}

void CTCPSocketManage::UserLogin(ClientUserInfo* userInfo, const char* szPassWord, int nNameID) // 登录G服务器
{
	MSG_GR_S_RoomLogon loginRoom;
	memset(&loginRoom, 0, sizeof(loginRoom));
	loginRoom.bForced = false;
	loginRoom.dwUserID = userInfo->nUserID;
	loginRoom.uNameID = nNameID;

	MD5_CTX Md5;
	Md5.MD5Update((unsigned char *)szPassWord, lstrlen(szPassWord));
	unsigned char szMDTemp[16];
	Md5.MD5Final(szMDTemp);
	char szMD5Input[64];
	for (int i = 0; i < 16; i++)
	{
		wsprintf(&szMD5Input[i * 2], "%02x", szMDTemp[i]);
	}
	strcpy(loginRoom.szMD5Pass, szMD5Input);
	userInfo->socketHandle->SendData(&loginRoom, sizeof(loginRoom), MDM_GR_LOGON, ASS_GR_LOGON_BY_ID); // 发送登录
	
}

void CTCPSocketManage::SendData(int nUserID, int nMainID, int nAssistID, int nLoopCount, LoginMG login) // 发送数据
{
	ClientUserInfo userInfo;
	userInfo.nUserID = nUserID;
	userInfo.socketHandle = nullptr;
	m_mutxUsers.lock();
	bool bRet = m_treeClientInfo.FindElement(userInfo);
	m_mutxUsers.unlock();
	if (!bRet)
		return;
	if (!userInfo.socketHandle)
		return;
	char szMessage[MaxBuffLen] = { 0x00 };
	if (login == LoginMG::LOGIN_M) // M
	{
		int nLen = 0;
		m_mXmlMParse.GetMessageBody(nMainID, nAssistID, szMessage, nLen);
		userInfo.socketHandle->SendData(szMessage, nLen, nMainID, nAssistID);
	}
	else // G
	{
		int nLen;
		m_mXmlGParse.GetMessageBody(nMainID, nAssistID, szMessage, nLen);
		userInfo.socketHandle->SendData(szMessage, nLen, nMainID, nAssistID);
	}
}

void CTCPSocketManage::Reload(LoginMG login) // 重新加载xml
{
	if (login == LoginMG::LOGIN_M)
		m_mXmlMParse.ReloadXml();
	else
		m_mXmlGParse.ReloadXml();
}

void CTCPSocketManage::SendDataBatch(int nMainID, int nAssistID, int nLoopCount, LoginMG login) // 批量发送数据 nLoopCount次
{
	char szMessage[MaxBuffLen] = { 0x00 };
	int nLen = 0;
	if (login == LoginMG::LOGIN_M)
		m_mXmlMParse.GetMessageBody(nMainID, nAssistID, szMessage, nLen);
	else
		m_mXmlGParse.GetMessageBody(nMainID, nAssistID, szMessage, nLen);
	for (int i = 0; i < nLoopCount; i++)
	{
		m_mutxUsers.lock();
		m_treeClientInfo.DoSomthing([&](ClientUserInfo& info) { info.socketHandle->SendData(szMessage, nLen, nMainID, nAssistID); });
		m_mutxUsers.unlock();
	}
}

void CTCPSocketManage::Quit()
{
	m_mutxUsers.lock();
	m_treeClientInfo.DoSomthing([](ClientUserInfo& info) { info.socketHandle->SetExit(true); }); // 不再投递接受消息
	m_mutxUsers.unlock();

	g_ThreadPool.Quit(); // 等待所有任务执行完毕
	CloseAll(); // 关闭所有socket
	m_mutxUsers.lock();
	m_treeClientInfo.Clear(); // 清除所有节点
	m_mutxUsers.unlock();
	if (m_threadHandleData.joinable())
		m_threadHandleData.join(); // 处理线程退出
	CLog::GetInstance()->Write("Quit...");
	CLog::GetInstance()->Exit();
}

void CTCPSocketManage::CloseAll()
{
	m_mutxUsers.lock();
	m_treeClientInfo.DoSomthing([](ClientUserInfo& info) { info.socketHandle->CloseSocket(); });
	m_mutxUsers.unlock();
}

void CTCPSocketManage::SetStart(bool bStart) // 未登录时的退出
{
	m_bStart = true;
	if (m_threadHandleData.joinable())
		m_threadHandleData.join();
}