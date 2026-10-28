#pragma once

#include "Types.h"

#include <winsock2.h>
#include <windows.h>

enum class IocpEventType : uint8
{
	None,
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

// 어떤 I/O가 완료되었는지 나타내는 이벤트다.
class IocpEvent : public OVERLAPPED
{
public:
	explicit IocpEvent(IocpEventType type);
	IocpEvent(const IocpEvent&) = delete;
	IocpEvent& operator=(const IocpEvent&) = delete;
	IocpEvent(IocpEvent&&) = delete;
	IocpEvent& operator=(IocpEvent&&) = delete;

	// 이전 I/O의 완료 처리 후 호출한다. 소유 참조는 유지하고 OVERLAPPED만 초기화한다.
	void			Init();

	IocpEventType	GetEventType() const					{ return _type; }
	IocpObjectRef	GetOwner() const						{ return _owner; }

	// I/O 요청 전에 설정하고, 즉시 실패해 완료가 오지 않는 경우 호출자가 해제한다.
	void			SetOwner(IocpObjectRef owner)			{ _owner = owner; }

private:
	IocpEventType	_type;
	IocpObjectRef	_owner;
};
