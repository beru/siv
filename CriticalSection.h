#pragma once

#include <Windows.h>
#include "noncopyable.h"

class CriticalSection : private noncopyable
{
public:
	CriticalSection() { InitializeCriticalSection(&cs_); }
	~CriticalSection() { DeleteCriticalSection(&cs_); }

	class Lock : private noncopyable {
	public:
		Lock(CriticalSection& cs) : cs_(cs) { cs_.Enter(); }
		~Lock() { cs_.Leave(); }
	private:
		CriticalSection& cs_;
	};

private: 
	void Enter() { EnterCriticalSection(&cs_); }
	void Leave() { LeaveCriticalSection(&cs_); }

	CRITICAL_SECTION cs_;
};

