#include "Log.h"
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <stdarg.h>
#include <ctime>

// »’÷æ¿‡

const char* logDir = "./log";
CLog* CLog::m_Instance = nullptr;

CLog::CLog()
{
}


CLog::~CLog()
{
}

CLog* CLog::GetInstance()
{
	if (m_Instance == nullptr)
		m_Instance = new CLog;
	return m_Instance;
}

void CLog::Init()
{
	m_fFile = NULL;
	if (_access(logDir, 0) != 0)
		_mkdir(logDir);
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	char szFileName[128] = { 0x00 };
	char szTime[64] = { 0x0 };
	sprintf(szTime, "%04d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	sprintf(szFileName, "%s/%s.txt", logDir, szTime);
	m_fFile = fopen(szFileName, "a+");
}

void CLog::Write(char* fmt, ...)
{
	if (m_fFile != NULL)
	{
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		char szTime[64] = { 0x0 };
		sprintf(szTime, "%d:%d:%d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		m_mtxFile.lock();
		va_list arg;
		va_start(arg, fmt);
		fprintf(m_fFile, "[%s]--", szTime);
		vfprintf(m_fFile, fmt, arg);
		fprintf(m_fFile, "\n");
		fflush(m_fFile);
		m_mtxFile.unlock();
	}
}

void CLog::Exit()
{
	if (m_fFile)
	{
		fclose(m_fFile);
		m_fFile = NULL;
	}
	if (m_Instance)
		delete m_Instance;
}