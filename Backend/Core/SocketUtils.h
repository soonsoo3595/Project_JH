#pragma once

#include "Core/StatusCode.h"
#include "Core/NetAddress.h"

#include <winsock2.h>

// WinsockRuntime 초기화에 성공한 뒤 사용한다.
class SocketUtils final
{
public:
	SocketUtils() = delete;

	// 출력 변수는 INVALID_SOCKET으로 전달한다. 생성한 소켓은 호출자가 소유한다.
	static StatusCode CreateTcpSocket(OUT SOCKET& socket);
	static StatusCode SetTcpNoDelay(SOCKET socket, bool isEnabled);

	// bind 전에 설정한다. 주소 독점 옵션과 동시에 활성화하지 않는다.
	static StatusCode SetReuseAddress(SOCKET socket, bool isEnabled);

	// bind 전에 설정한다.
	static StatusCode SetExclusiveAddressUse(SOCKET socket, bool isEnabled);
	static StatusCode SetSendBufferSize(SOCKET socket, int32 size);
	static StatusCode SetReceiveBufferSize(SOCKET socket, int32 size);

	static StatusCode Bind(SOCKET socket, const NetAddress& address);
	static StatusCode Listen(SOCKET socket, int32 backlog = SOMAXCONN);

	static StatusCode GetLocalAddress(SOCKET socket, OUT NetAddress& address);

	// 성공 시 INVALID_SOCKET으로 되돌린다. 이미 무효라면 아무 작업도 하지 않는다.
	static StatusCode CloseSocket(OUT SOCKET& socket);
};
