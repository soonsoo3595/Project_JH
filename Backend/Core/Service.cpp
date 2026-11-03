#include "Core/Service.h"

#include "Core/IocpCore.h"
#include "Core/Session.h"
#include "Core/Listener.h"
#include "Core/Logger.h"

#include <cassert>

Service::Service(ServiceType type, NetAddress address, int32 maxSessionCount) 
	: _type(type), _address(address), _maxSessionCount(maxSessionCount)
{
	assert(_maxSessionCount > 0);
	_sessions.reserve(static_cast<std::size_t>(_maxSessionCount));

	_iocpCore = std::make_shared<IocpCore>();
	_sessionFactory = []() -> SessionRef
		{
			return std::make_shared<Session>();
		};
}

StatusCode Service::Start()
{
	if (_iocpCore == nullptr || _iocpCore->IsInitialized() == false)
	{
		return StatusCode::SC_IOCP_NOT_INITIALIZED;
	}

	if (_sessionFactory == nullptr)
	{
		return StatusCode::SC_SERVICE_INVALID_SESSION_FACTORY;
	}

	return StatusCode::SC_SUCCESS;
}

StatusCode Service::Stop()
{
	// TODO: 잠금 안에서 세션 목록을 복사하고, 잠금 해제 후 각 세션에 종료 요청.
	return StatusCode::SC_SUCCESS;
}

StatusCode Service::Register(IocpObjectRef iocpObject) const
{
	assert(iocpObject != nullptr);
	return _iocpCore->Register(iocpObject);
}

StatusCode Service::Dispatch(uint32 timeoutMs) const
{
	assert(_iocpCore != nullptr);
	return _iocpCore->Dispatch(timeoutMs);
}

StatusCode Service::CreateSession(OUT SessionRef& session)
{
	if (_iocpCore == nullptr || _iocpCore->IsInitialized() == false)
	{
		return StatusCode::SC_IOCP_NOT_INITIALIZED;
	}

	if (_sessionFactory == nullptr)
	{
		return StatusCode::SC_SERVICE_INVALID_SESSION_FACTORY;
	}

	SessionRef createdSession = _sessionFactory();
	if (createdSession == nullptr)
	{
		return StatusCode::SC_SERVICE_SESSION_CREATE_FAILED;
	}
	createdSession->SetService(shared_from_this());

	const StatusCode result = Register(createdSession);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	session = createdSession;
	return StatusCode::SC_SUCCESS;
}

StatusCode Service::AddSession(SessionRef session)
{
	if (session == nullptr)
	{
		return StatusCode::SC_SERVICE_INVALID_SESSION;
	}

	const uint64 sessionId = session->GetSessionId();

	int32 currentSessionCount = 0;
	{
		LockGuard lock(_mutex);
		if (_sessions.find(sessionId) != _sessions.end())
		{
			return StatusCode::SC_SUCCESS;
		}

		if (_sessions.size() >= static_cast<std::size_t>(_maxSessionCount))
		{
			return StatusCode::SC_SERVICE_SESSION_LIMIT_REACHED;
		}

		_sessions.emplace(sessionId, session);
		currentSessionCount = static_cast<int32>(_sessions.size());
	}

	LOG_INFO("세션 추가: %d / %d", currentSessionCount, _maxSessionCount);
	return StatusCode::SC_SUCCESS;
}

StatusCode Service::RemoveSession(SessionRef session)
{
	if (session == nullptr)
	{
		return StatusCode::SC_SERVICE_INVALID_SESSION;
	}

	int32 currentSessionCount = 0;
	{
		LockGuard lock(_mutex);
		if (_sessions.erase(session->GetSessionId()) == 0)
		{
			return StatusCode::SC_SERVICE_SESSION_NOT_FOUND;
		}
		currentSessionCount = static_cast<int32>(_sessions.size());
	}

	LOG_INFO("세션 삭제: %d / %d", currentSessionCount, _maxSessionCount);
	return StatusCode::SC_SUCCESS;
}

ServerService::ServerService(NetAddress targetAddress, int32 maxSessionCount)
	: Service(ServiceType::Server, targetAddress, maxSessionCount)
{
}

StatusCode ServerService::Start()
{
	const StatusCode result = Service::Start();
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}
	
	_listener = std::make_shared<Listener>();

	// Listener가 Listen을 성공적으로 시작했는지 확인
	const StatusCode listenerResult = _listener->Start(static_pointer_cast<ServerService>(shared_from_this()));
	if (listenerResult != StatusCode::SC_SUCCESS)
	{
		(void)Stop();
		_listener.reset();

		return listenerResult;
	}

	return StatusCode::SC_SUCCESS;
}

StatusCode ServerService::Stop()
{
	if (_listener == nullptr)
	{
		return StatusCode::SC_SUCCESS;
	}

	const StatusCode result = _listener->Close();
	if (result != StatusCode::SC_SUCCESS)
	{
		const int nativeError = ::WSAGetLastError();
		LOG_ERROR("리스너 종료 실패: %s (Winsock 오류: %d)", GetStatusMessage(result), nativeError);
	}

	return result;
}

ClientService::ClientService(NetAddress targetAddress, int32 maxSessionCount)
	: Service(ServiceType::Client, targetAddress, maxSessionCount)
{
}

StatusCode ClientService::Start()
{
	const StatusCode result = Service::Start();
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	return StatusCode::SC_SUCCESS;
}
