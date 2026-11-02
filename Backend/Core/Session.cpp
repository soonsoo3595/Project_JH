#include "Core/Session.h"

#include "Core/SocketUtils.h"
#include "Core/Logger.h"

Session::Session()
{
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
