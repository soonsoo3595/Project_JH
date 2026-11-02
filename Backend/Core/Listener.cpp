#include "Core/Listener.h"

#include "Core/SocketUtils.h"
#include "Core/Service.h"
#include "Core/IocpEvent.h"
#include "Core/Session.h"
#include "Core/Logger.h"

Listener::~Listener()
{
	const StatusCode result = SocketUtils::CloseSocket(_socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		const int nativeError = ::WSAGetLastError();
		LOG_ERROR("리스너 소켓 해제 실패: %s (Winsock 오류: %d)", GetStatusMessage(result), nativeError);
	}
}

StatusCode Listener::Start(shared_ptr<ServerService> service)
{
	assert(service != nullptr);
	_service = service;

	shared_ptr<ServerService> servicePtr = _service.lock();
	StatusCode result = StatusCode::SC_SUCCESS;

	result = SocketUtils::CreateTcpSocket(_socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	result = servicePtr->Register(shared_from_this());
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	result = SocketUtils::SetReuseAddress(_socket, true);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	result = SocketUtils::SetTcpNoDelay(_socket, true);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}
	
	result = SocketUtils::Bind(_socket, servicePtr->GetAddress());
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	result = SocketUtils::Listen(_socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	const int32 maxSessionCount = servicePtr->GetMaxSessionCount();
	_acceptEvents.reserve(maxSessionCount);

	for (int i = 0; i < maxSessionCount; ++i)
	{
		// TODO : AcceptEvent 생성 실패 시 어떻게 처리할지
		// maxSessionCount만큼 생성을 반드시 해야 하는지, 실패 시 바로 종료할지, 아니면 일부만 생성하고 계속 진행할지
		AcceptEvent* acceptEvent = new AcceptEvent();
		if (acceptEvent == nullptr)
		{

		}

		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return StatusCode::SC_SUCCESS;
}

StatusCode Listener::Close()
{
	_isClosing.store(true);
	return SocketUtils::CloseSocket(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, DWORD numOfBytes, DWORD error)
{
	assert(iocpEvent != nullptr && iocpEvent->GetEventType() == IocpEventType::Accept);
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);

	// 종료 중에는 성공 완료도 등록하지 않고 수락용 세션을 정리한다.
	if (_isClosing.load() == true)
	{
		acceptEvent->session.reset();
		return;
	}

	if (error != ERROR_SUCCESS)
	{
		acceptEvent->session.reset();
		LOG_WARNING("연결 수락 완료 실패 (Windows 오류: %lu)", error);
		if (error != ERROR_OPERATION_ABORTED)
		{
			RegisterAccept(acceptEvent);
		}
		return;
	}

	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	// TODO: 즉시 실패 시 재귀 재시도를 참조 정리 및 타이머·작업 큐 기반 지연 재시도로 교체.
	assert(acceptEvent != nullptr);

	if (_isClosing.load() == true)
	{
		acceptEvent->session.reset();
		return;
	}

	shared_ptr<ServerService> servicePtr = _service.lock();
	if (servicePtr == nullptr)
	{
		return;
	}

	StatusCode result;
	SessionRef session;

	result = servicePtr->CreateSession(session);
	if (result != StatusCode::SC_SUCCESS)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	acceptEvent->Init();
	acceptEvent->SetOwner(shared_from_this());
	acceptEvent->session = session;

	// TODO: 버퍼 클래스 구현 시 Accept 완료까지 유지되는 전용 주소 버퍼를 세 번째 인자로 전달.
	DWORD bytesReceived = 0;
	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), acceptEvent, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int error = ::WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			RegisterAccept(acceptEvent);
			return;
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	assert(acceptEvent != nullptr);

	shared_ptr<ServerService> servicePtr = _service.lock();
	if (servicePtr == nullptr)
	{
		return;
	}

	SessionRef session = acceptEvent->session;
	acceptEvent->session.reset();

	if (session == nullptr)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	const StatusCode result = SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		const int nativeError = ::WSAGetLastError();
		LOG_ERROR("Accept 컨텍스트 갱신 실패: %s (Winsock 오류: %d)", GetStatusMessage(result), nativeError);
		acceptEvent->session.reset();
		session.reset();
		RegisterAccept(acceptEvent);
		return;
	}

	const StatusCode addResult = servicePtr->AddSession(session);
	if (addResult != StatusCode::SC_SUCCESS)
	{
		LOG_WARNING("수락 세션 등록 실패: %s (상태 코드: %d)", GetStatusMessage(addResult), static_cast<int>(addResult));
		session.reset();
		RegisterAccept(acceptEvent);
		return;
	}

	RegisterAccept(acceptEvent);
}
