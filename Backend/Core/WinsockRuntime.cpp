#include "Core/WinsockRuntime.h"

WinsockRuntime::WinsockRuntime()
	: _startupError(0)
{
	WSADATA winsockData{};
	_startupError = WSAStartup(MAKEWORD(2, 2), &winsockData);
}

WinsockRuntime::~WinsockRuntime()
{
	if (_startupError == 0)
	{
		WSACleanup();
	}
}

int WinsockRuntime::GetStartupError() const
{
	return _startupError;
}

