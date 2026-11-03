#pragma once

#include "Types.h"
#include "StatusCode.h"
#include "Container.h"
#include "NetAddress.h"

#include <windows.h>
#include <functional>

enum class ServiceType : uint8
{
	None,
	Server,
	Client
};

using SessionFactory = std::function<SessionRef(void)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress address, int32 maxSessionCount = 1);
	virtual ~Service() = default;

	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;
	Service(Service&&) = delete;
	Service& operator=(Service&&) = delete;

public:
	virtual StatusCode  Start();
	virtual StatusCode  Stop();

	StatusCode			Register(IocpObjectRef iocpObject) const;
	StatusCode			Dispatch(uint32 timeoutMs = INFINITE) const;

	StatusCode			CreateSession(OUT SessionRef& session);
	StatusCode			AddSession(SessionRef session);
	StatusCode				RemoveSession(SessionRef session);

public:
	ServiceType			GetType() const noexcept			{ return _type; }
	NetAddress			GetAddress() const noexcept			{ return _address; }
	int32 				GetMaxSessionCount() const noexcept { return _maxSessionCount; }

protected:
	ServiceType			_type = ServiceType::None;
	NetAddress 			_address{};
	IocpCoreRef			_iocpCore;
	Mutex				_mutex;

protected:
	SessionFactory					_sessionFactory;
	HashMap<uint64, SessionRef>		_sessions;
	int32							_maxSessionCount = 0;
};

class ServerService : public Service
{
public:
	ServerService(NetAddress targetAddress, int32 maxSessionCount = 1);

	virtual StatusCode Start() override;
	virtual StatusCode Stop() override;

private:
	ListenerRef	_listener = nullptr;
};

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, int32 maxSessionCount = 1);

	virtual StatusCode Start() override;
};
