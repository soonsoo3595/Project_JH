#pragma once

#include "Types.h"
#include "Core/IocpCore.h"

class Service;
class IocpEvent;

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

	std::shared_ptr<Service>	GetService() const noexcept { return _service.lock(); }
	void						SetService(std::shared_ptr<Service> service) noexcept { _service = service; }

	SOCKET						GetSocket() const noexcept { return _socket; }

	uint64 GetSessionId() const noexcept { return _sessionId; }

public:
	virtual HANDLE	GetHandle() override { return reinterpret_cast<HANDLE>(_socket); }
	virtual void	Dispatch(IocpEvent* iocpEvent, DWORD numOfBytes = 0, DWORD error = ERROR_SUCCESS) override;

private:
	uint64					_sessionId = 0;
	std::weak_ptr<Service>	_service;
	SOCKET					_socket = INVALID_SOCKET;
	Atomic<bool> 			_isConnected = false;
};
