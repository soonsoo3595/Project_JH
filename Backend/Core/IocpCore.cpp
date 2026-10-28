#include "Core/IocpCore.h"

#include "Core/IocpEvent.h"

IocpCore::IocpCore()
{
	(void)Init();
}

IocpCore::~IocpCore()
{
	(void)Close();
}

StatusCode IocpCore::Register(IocpObjectRef iocpObject) const
{
	if (_handle == nullptr)
	{
		return StatusCode::SC_IOCP_NOT_INITIALIZED;
	}
	if (iocpObject == nullptr)
	{
		return StatusCode::SC_IOCP_INVALID_OBJECT;
	}
	const HANDLE handle = iocpObject->GetHandle();
	if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	if (::CreateIoCompletionPort(handle, _handle, 0, 0) == nullptr)
	{
		return StatusCode::SC_IOCP_REGISTER_FAILED;
	}

	return StatusCode::SC_SUCCESS;
}

StatusCode IocpCore::Dispatch(uint32 timeoutMs) const
{
	if (_handle == nullptr)
	{
		return StatusCode::SC_IOCP_NOT_INITIALIZED;
	}

	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	OVERLAPPED* overlapped = nullptr;
	const BOOL succeeded = ::GetQueuedCompletionStatus(_handle, OUT &numOfBytes, OUT &key,
		OUT &overlapped, timeoutMs);
	const DWORD error = succeeded != FALSE ? ERROR_SUCCESS : ::GetLastError();

	if (overlapped == nullptr)
	{
		// 수동 알림은 이벤트가 없을 수 있다. 패킷 없는 실패와 구분한다.
		if (succeeded != FALSE)
		{
			return StatusCode::SC_SUCCESS;
		}
		return error == WAIT_TIMEOUT ? StatusCode::SC_IOCP_WAIT_TIMEOUT : StatusCode::SC_IOCP_WAIT_FAILED;
	}

	// FALSE여도 이벤트가 있으면 실패한 I/O의 완료이므로 소유 객체에 전달한다.
	IocpEvent* iocpEvent = static_cast<IocpEvent*>(overlapped);
	IocpObjectRef iocpObject = iocpEvent->GetOwner();
	if (iocpObject == nullptr)
	{
		return StatusCode::SC_IOCP_INVALID_OBJECT;
	}

	// 지역 참조로 콜백 동안 수명을 보장한다. 콜백은 새 I/O의 소유자를 다시 설정할 수 있다.
	iocpEvent->SetOwner(nullptr);
	iocpObject->Dispatch(iocpEvent, numOfBytes, error);
	return succeeded != FALSE ? StatusCode::SC_SUCCESS : StatusCode::SC_IOCP_IO_FAILED;
}

StatusCode IocpCore::Init(uint32 concurrentThreadCount)
{
	if (_handle != nullptr)
	{
		return StatusCode::SC_IOCP_ALREADY_INITIALIZED;
	}
	const HANDLE handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrentThreadCount);
	if (handle == nullptr)
	{
		return StatusCode::SC_IOCP_CREATE_FAILED;
	}
	_handle = handle;
	return StatusCode::SC_SUCCESS;
}

StatusCode IocpCore::Close()
{
	if (_handle == nullptr)
	{
		return StatusCode::SC_SUCCESS;
	}
	if (CloseHandle(_handle) == FALSE)
	{
		return StatusCode::SC_IOCP_CLOSE_FAILED;
	}
	_handle = nullptr;
	return StatusCode::SC_SUCCESS;
}
