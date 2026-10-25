#include "Core/NetAddress.h"

#include <ws2tcpip.h>

NetAddress::NetAddress() noexcept
{
	_address.sin_family = AF_INET;
}

StatusCode NetAddress::SetAddress(const String& ip, uint16 port) noexcept
{
	if (ip.empty() || ip.size() >= INET_ADDRSTRLEN || ip.find('\0') != String::npos)
	{
		return StatusCode::SC_NET_INVALID_ADDRESS;
	}

	IN_ADDR parsedIp{};
	if (InetPtonA(AF_INET, ip.c_str(), &parsedIp) != 1)
	{
		return StatusCode::SC_NET_INVALID_ADDRESS;
	}
	_address.sin_addr = parsedIp;
	_address.sin_port = htons(port);
	return StatusCode::SC_SUCCESS;
}

StatusCode NetAddress::SetAddress(const sockaddr_in& address) noexcept
{
	if (address.sin_family != AF_INET)
	{
		return StatusCode::SC_NET_INVALID_ADDRESS;
	}
	_address = address;
	return StatusCode::SC_SUCCESS;
}

String NetAddress::GetIp() const
{
	char text[INET_ADDRSTRLEN]{};
	if (InetNtopA(AF_INET, &_address.sin_addr, text, sizeof(text)) == nullptr)
	{
		return {};
	}
	return text;
}