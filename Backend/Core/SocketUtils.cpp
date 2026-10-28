#include "Core/SocketUtils.h"

static StatusCode SetSocketOption(SOCKET socket, int level, int name, const void* value, int size)
{
	if (socket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	if (setsockopt(socket, level, name, static_cast<const char*>(value), size) == SOCKET_ERROR)
	{
		return StatusCode::SC_NET_SOCKET_OPTION_FAILED;
	}
	return StatusCode::SC_SUCCESS;
}

StatusCode SocketUtils::CreateTcpSocket(OUT SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		return StatusCode::SC_NET_SOCKET_ALREADY_OPEN;
	}
	// IOCP에 사용할 수 있는 소켓을 만들고 자식 프로세스에 핸들이 상속되지 않도록 한다.
	const SOCKET createdSocket = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0,
		WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
	if (createdSocket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_SOCKET_CREATE_FAILED;
	}
	socket = createdSocket;
	return StatusCode::SC_SUCCESS;
}

StatusCode SocketUtils::SetTcpNoDelay(SOCKET socket, bool isEnabled)
{
	const BOOL value = isEnabled ? TRUE : FALSE;
	return SetSocketOption(socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}

StatusCode SocketUtils::SetExclusiveAddressUse(SOCKET socket, bool isEnabled)
{
	const BOOL value = isEnabled ? TRUE : FALSE;
	return SetSocketOption(socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &value, sizeof(value));
}

StatusCode SocketUtils::SetReuseAddress(SOCKET socket, bool isEnabled)
{
	const BOOL value = isEnabled ? TRUE : FALSE;
	return SetSocketOption(socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
}

StatusCode SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	if (size <= 0)
	{
		return StatusCode::SC_NET_INVALID_ARGUMENT;
	}
	return SetSocketOption(socket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

StatusCode SocketUtils::SetReceiveBufferSize(SOCKET socket, int32 size)
{
	if (size <= 0)
	{
		return StatusCode::SC_NET_INVALID_ARGUMENT;
	}
	return SetSocketOption(socket, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

StatusCode SocketUtils::Bind(SOCKET socket, const NetAddress& address)
{
	if (socket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	const sockaddr_in& nativeAddress = address.GetSockAddr();
	if (::bind(socket, reinterpret_cast<const sockaddr*>(&nativeAddress), sizeof(nativeAddress)) == SOCKET_ERROR)
	{
		return StatusCode::SC_NET_BIND_FAILED;
	}
	return StatusCode::SC_SUCCESS;
}

StatusCode SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	if (socket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	if (backlog <= 0)
	{
		return StatusCode::SC_NET_INVALID_ARGUMENT;
	}
	if (listen(socket, backlog) == SOCKET_ERROR)
	{
		return StatusCode::SC_NET_LISTEN_FAILED;
	}
	return StatusCode::SC_SUCCESS;
}

StatusCode SocketUtils::GetLocalAddress(SOCKET socket, OUT NetAddress& address)
{
	if (socket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	sockaddr_in nativeAddress{};
	int size = sizeof(nativeAddress);
	if (getsockname(socket, reinterpret_cast<sockaddr*>(&nativeAddress), &size) == SOCKET_ERROR)
	{
		return StatusCode::SC_NET_LOCAL_ADDRESS_FAILED;
	}
	return address.SetAddress(nativeAddress);
}

StatusCode SocketUtils::CloseSocket(OUT SOCKET& socket)
{
	if (socket == INVALID_SOCKET)
	{
		return StatusCode::SC_SUCCESS;
	}
	if (closesocket(socket) == SOCKET_ERROR)
	{
		return StatusCode::SC_NET_SOCKET_CLOSE_FAILED;
	}
	socket = INVALID_SOCKET;
	return StatusCode::SC_SUCCESS;
}
