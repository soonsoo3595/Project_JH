#pragma once

#include "Core/StatusCode.h"

#include <winsock2.h>
#include <windows.h>

// IOCP에 등록될 수 있는 객체
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual ~IocpObject() = default;
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IocpEvent* iocpEvent, DWORD numOfBytes = 0, DWORD error = ERROR_SUCCESS) = 0;
};

class IocpCore final
{
public:
	IocpCore();
	~IocpCore();

	IocpCore(const IocpCore&) = delete;
	IocpCore& operator=(const IocpCore&) = delete;
	IocpCore(IocpCore&&) = delete;
	IocpCore& operator=(IocpCore&&) = delete;

	bool IsInitialized() const noexcept { return _handle != INVALID_HANDLE_VALUE; }

	StatusCode Register(IocpObjectRef iocpObject) const;
	StatusCode Dispatch(uint32 timeoutMs = INFINITE) const;

private:
	StatusCode Init(uint32 concurrentThreadCount = 0);
	StatusCode Close();

private:
	HANDLE _handle = INVALID_HANDLE_VALUE;
};
