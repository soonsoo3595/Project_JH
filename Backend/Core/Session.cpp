#include "Core/Session.h"

#include "Core/SocketUtils.h"
#include "Core/Logger.h"

Session::Session()
{
	// 프로세스 안에서 생성마다 한 번 발급하며 삭제된 ID는 재사용하지 않는다.
	static Atomic<uint64> nextSessionId{1};
	_sessionId = nextSessionId.fetch_add(1);

	const StatusCode result = SocketUtils::CreateTcpSocket(_socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		const int nativeError = ::WSAGetLastError();
		LOG_ERROR("세션 소켓 생성 실패: %s (Winsock 오류: %d)", GetStatusMessage(result), nativeError);
	}
}

Session::~Session()
{
	const StatusCode result = SocketUtils::CloseSocket(_socket);
	if (result != StatusCode::SC_SUCCESS)
	{
		const int nativeError = ::WSAGetLastError();
		LOG_ERROR("세션 소켓 해제 실패: %s (Winsock 오류: %d)", GetStatusMessage(result), nativeError);
	}
}

void Session::Dispatch(IocpEvent* iocpEvent, DWORD numOfBytes, DWORD error)
{
}
