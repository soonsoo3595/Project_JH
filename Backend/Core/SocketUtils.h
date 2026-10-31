#pragma once

#include "Core/StatusCode.h"
#include "Core/NetAddress.h"

#include <winsock2.h>
#include <mswsock.h>

class SocketUtils final
{
public:
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	SocketUtils() = delete;
	static StatusCode BindWindowsFunction(SOCKET socket, GUID guid, OUT LPVOID* fn);

	static StatusCode CreateTcpSocket(OUT SOCKET& socket);
	static StatusCode SetTcpNoDelay(SOCKET socket, bool isEnabled);
	static StatusCode SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	// bind 전에 설정한다.
	static StatusCode SetReuseAddress(SOCKET socket, bool isEnabled);
	static StatusCode SetExclusiveAddressUse(SOCKET socket, bool isEnabled);
	static StatusCode SetSendBufferSize(SOCKET socket, int32 size);
	static StatusCode SetReceiveBufferSize(SOCKET socket, int32 size);

	static StatusCode Bind(SOCKET socket, const NetAddress& address);
	static StatusCode Listen(SOCKET socket, int32 backlog = SOMAXCONN);

	static StatusCode GetLocalAddress(SOCKET socket, OUT NetAddress& address);
	static StatusCode CloseSocket(OUT SOCKET& socket);

private:
	// Init은 WinsockRuntime에서만 호출한다
	friend class WinsockRuntime;
	static StatusCode Init();
};
