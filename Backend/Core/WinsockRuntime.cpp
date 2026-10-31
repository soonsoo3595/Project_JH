#include "Core/WinsockRuntime.h"

#include "Core/SocketUtils.h"

WinsockRuntime::WinsockRuntime()
{
	WSADATA winsockData{};
	_startupResult = ::WSAStartup(MAKEWORD(2, 2), OUT &winsockData);
	if (_startupResult != 0)
	{
		_socketInitResult = StatusCode::SC_NET_WINSOCK_STARTUP_FAILED;
		_winsockError = _startupResult;
		return;
	}

	_socketInitResult = SocketUtils::Init();
	if (_socketInitResult != StatusCode::SC_SUCCESS)
	{
		_winsockError = ::WSAGetLastError();
	}
}

WinsockRuntime::~WinsockRuntime()
{
	if (_startupResult == 0)
	{
		::WSACleanup();
	}
}
