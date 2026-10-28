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
	// 실패·취소된 I/O도 전달한다. error가 ERROR_SUCCESS가 아니면 실패 완료다.
	virtual void Dispatch(class IocpEvent* iocpEvent, DWORD numOfBytes = 0,
		DWORD error = ERROR_SUCCESS) = 0;
};

class IocpCore final
{
public:
	// 생성 후 IsInitialized()를 확인한다. 모든 완료 처리·워커 합류 후 소멸시킨다.
	IocpCore();
	~IocpCore();

	IocpCore(const IocpCore&) = delete;
	IocpCore& operator=(const IocpCore&) = delete;
	IocpCore(IocpCore&&) = delete;
	IocpCore& operator=(IocpCore&&) = delete;

	bool IsInitialized() const noexcept { return _handle != nullptr; }

	StatusCode Register(IocpObjectRef iocpObject) const;
	// 대기 중인 I/O마다 유효한 IocpEvent와 소유 객체를 유지해야 한다.
	StatusCode Dispatch(uint32 timeoutMs = INFINITE) const;

private:
	StatusCode Init(uint32 concurrentThreadCount = 0);
	StatusCode Close();

private:
	HANDLE _handle = nullptr;
};
