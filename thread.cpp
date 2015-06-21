
#include "thread.h"

#include <cassert>

Thread::Thread()
	:
	hThread_(nullptr),
	dwThreadId_(0)
{
}

Thread::~Thread()
{
	if (hThread_ != nullptr) {
		CloseHandle(hThread_);
		hThread_ = nullptr;
	}
}
	
void Thread::Create(LPTHREAD_START_ROUTINE pThreadProc,
					LPVOID pParam, // = nullptr,
					DWORD dwCreationFlags, // = 0,
					LPSECURITY_ATTRIBUTES pSecurityAttr, // = nullptr,
					DWORD dwStackSize // = 0
					)
{
	DWORD dwThreadId = 0;
	hThread_ = (HANDLE) _beginthreadex(pSecurityAttr,
									   dwStackSize,
									   (unsigned (__stdcall*)(void*)) pThreadProc,
									   pParam,
									   dwCreationFlags,
									   (unsigned*) &dwThreadId_
									   );
}
	
int Thread::GetPriority() const
{
	assert(hThread_ != nullptr);
	return GetThreadPriority(hThread_);
}

BOOL Thread::SetPriority(int nPriority)
{
	assert(hThread_ != nullptr);
	return SetThreadPriority(hThread_, nPriority);
}

DWORD Thread::GetExitCode() const
{
	assert(hThread_ != nullptr);
	DWORD dwExitCode = 0;
	if (GetExitCodeThread(hThread_, &dwExitCode)) {
		return dwExitCode;
	}else {
		return (DWORD) -1;
	}
}

BOOL Thread::GetThreadTimes(LPFILETIME pCreationTime,
							LPFILETIME pExitTime,
							LPFILETIME pKernelTime,
							LPFILETIME pUserTime
							) const
{
	assert(hThread_ != nullptr);
	return ::GetThreadTimes(hThread_,
							pCreationTime,
							pExitTime,
							pKernelTime,
							pUserTime);
}

#if _WIN32_WINNT >= 0x0501
BOOL Thread::IsIOPending() const
{
	assert(hThread_ != nullptr);
	BOOL bIOPending = FALSE;
	GetThreadIOPendingFlag(hThread_, &bIOPending);
	return bIOPending;
}
#endif

DWORD Thread::Resume()
{
	assert(hThread_ != nullptr);
	return ResumeThread(hThread_);
}

DWORD Thread::Suspend()
{
	assert(hThread_ != nullptr);
	return SuspendThread(hThread_);
}

BOOL Thread::Terminate(DWORD dwExitCode /* = 0 */)
{
	assert(hThread_ != nullptr);
	return TerminateThread(hThread_, dwExitCode);
}

void Thread::Exit(DWORD dwExitCode /* = 0 */)
{
	// Make sure this is only called from the thread that this object represents
	assert( dwThreadId_ == ::GetCurrentThreadId() );

	_endthreadex(dwExitCode);
}

DWORD Thread::Join(DWORD dwWaitMilliseconds /* = INFINITE */)
{
	assert(hThread_ != nullptr);
	DWORD ret = WaitForSingleObject(hThread_, dwWaitMilliseconds);
	hThread_ = 0;
	return ret;
}

