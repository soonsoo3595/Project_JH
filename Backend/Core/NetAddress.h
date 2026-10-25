#pragma once

#include "Core/Container.h"
#include "Core/StatusCode.h"
#include "Core/Types.h"

#include <winsock2.h>

// IPv4 전용이다. 기본값은 모든 로컬 인터페이스·자동 할당 포트(0.0.0.0:0)다.
class NetAddress final
{
public:
	NetAddress() noexcept;

	// 숫자 IPv4 주소만 받는다. 실패하면 기존 주소를 유지한다.
	StatusCode SetAddress(const String& ip, uint16 port) noexcept;
	StatusCode SetAddress(const sockaddr_in& address) noexcept;

	String GetIp() const;
	uint16 GetPort() const noexcept					{ return ntohs(_address.sin_port); }
	const sockaddr_in& GetSockAddr() const noexcept { return _address; }

private:
	// {} 초기화 시 모든 멤버가 0으로 초기화되어 memset 사용 안해도 됨
	sockaddr_in		_address{};
};
