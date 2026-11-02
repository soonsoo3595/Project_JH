#pragma once

#include "IocpCore.h"
#include "Types.h"

class AcceptEvent;
class ServerService;

using namespace std;

class Listener : public IocpObject
{
public:
	Listener() = default;
	virtual ~Listener();

public:
	StatusCode Start(shared_ptr<ServerService> service);
	StatusCode Close();

public:
	HANDLE GetHandle() override { return reinterpret_cast<HANDLE>(_socket); }
	void Dispatch(IocpEvent* iocpEvent, DWORD numOfBytes, DWORD error) override;

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

private:
	SOCKET							_socket = INVALID_SOCKET;
	weak_ptr<class ServerService>	_service;
	Vector<AcceptEvent*>			_acceptEvents;
	Atomic<bool>					_isClosing = false;
};
