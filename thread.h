#pragma once

#include <Windows.h>
#include <process.h>
#include "noncopyable.h"

struct Thread : private noncopyable
{
	Thread();
	virtual ~Thread();
	
	void Create(LPTHREAD_START_ROUTINE pThreadProc,
				LPVOID pParam = nullptr,
				DWORD dwCreationFlags = 0,
				LPSECURITY_ATTRIBUTES pSecurityAttr = nullptr,
				DWORD dwStackSize = 0
				);
	
	int GetPriority() const;
	BOOL SetPriority(int nPriority);
	DWORD GetExitCode() const;
	BOOL GetThreadTimes(LPFILETIME pCreationTime,
						LPFILETIME pExitTime,
						LPFILETIME pKernelTime,
						LPFILETIME pUserTime
						) const;
#if _WIN32_WINNT >= 0x0501
	BOOL IsIOPending() const;
#endif
	DWORD Resume();
	DWORD Suspend();
	BOOL Terminate(DWORD dwExitCode = 0);
	void Exit(DWORD dwExitCode = 0);
	DWORD Join(DWORD dwWaitMilliseconds = INFINITE);
	
	HANDLE hThread_;
	DWORD dwThreadId_;
};

