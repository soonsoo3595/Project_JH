#include "Core/SocketUtils.h"

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

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

StatusCode SocketUtils::Init()
{
	SOCKET dummySocket = INVALID_SOCKET;
	StatusCode result = CreateTcpSocket(dummySocket);
	if (result != StatusCode::SC_SUCCESS)
	{
		return result;
	}

	LPFN_CONNECTEX connectEx = nullptr;
	LPFN_DISCONNECTEX disconnectEx = nullptr;
	LPFN_ACCEPTEX acceptEx = nullptr;
	result = BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&connectEx));
	if (result == StatusCode::SC_SUCCESS)
	{
		result = BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&disconnectEx));
	}
	if (result == StatusCode::SC_SUCCESS)
	{
		result = BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&acceptEx));
	}

	// 임시 소켓 정리로 초기화 실패의 원본 오류가 덮어써지지 않도록 보존한다.
	const int nativeError = result != StatusCode::SC_SUCCESS ? ::WSAGetLastError() : 0;
	const StatusCode closeResult = CloseSocket(dummySocket);
	if (result != StatusCode::SC_SUCCESS)
	{
		::WSASetLastError(nativeError);
		return result;
	}
	if (closeResult != StatusCode::SC_SUCCESS)
	{
		return closeResult;
	}

	// 전체 초기화가 성공했을 때만 공개 함수 포인터에 반영한다.
	ConnectEx = connectEx;
	DisconnectEx = disconnectEx;
	AcceptEx = acceptEx;
	return StatusCode::SC_SUCCESS;
}

StatusCode SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, OUT LPVOID* fn)
{
	DWORD bytes = 0;
	if (SOCKET_ERROR == ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT &bytes, nullptr, nullptr))
	{
		return StatusCode::SC_NET_EXTENSION_FUNCTION_BIND_FAILED;
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

// TCP_NODELAY 옵션은 Nagle 알고리즘을 끄는 옵션 (기본값 : 켜짐)
StatusCode SocketUtils::SetTcpNoDelay(SOCKET socket, bool isEnabled)
{
	const BOOL value = isEnabled ? TRUE : FALSE;
	return SetSocketOption(socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}

// AcceptEx 성공 완료 후 수락 소켓에 리슨 소켓의 컨텍스트를 반영한다.
StatusCode SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	if (listenSocket == INVALID_SOCKET)
	{
		return StatusCode::SC_NET_INVALID_SOCKET;
	}
	return SetSocketOption(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, &listenSocket, sizeof(listenSocket));
}

// SO_EXCLUSIVEADDRUSE는 bind 전에 설정하며 다른 소켓의 동일 주소·포트 강제 바인딩을 막는다. (기본값: 꺼짐)
StatusCode SocketUtils::SetExclusiveAddressUse(SOCKET socket, bool isEnabled)
{
	const BOOL value = isEnabled ? TRUE : FALSE;
	return SetSocketOption(socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &value, sizeof(value));
}

// SO_REUSEADDR 옵션은 bind 시 다른 소켓이 같은 주소를 bind할 수 있도록 한다. (기본값 : 꺼짐)
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
