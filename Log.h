#define _CRT_SECURE_NO_WARNINGS
#ifndef __CLOG__H
#define __CLOG__H
#include <stdio.h>
#include <mutex>
#include <iostream>

class CLog
{
private:
	CLog();
	~CLog();
public:
	static CLog* GetInstance();
	void Init();
	void Write(char* fmt, ...);
	void Exit();
private:
	static CLog* m_Instance;
	FILE* m_fFile; // ÎÄ¼þÖ¸Õë
	std::mutex m_mtxFile;
};

#endif
