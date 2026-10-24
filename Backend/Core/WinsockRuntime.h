#pragma once

// 인스턴스 수명 동안 Winsock을 유지한다. 이후 생성할 소켓보다 먼저 생성하고 나중에 파괴한다.
class WinsockRuntime final
{
public:
	WinsockRuntime();
	~WinsockRuntime();

	WinsockRuntime(const WinsockRuntime&) = delete;
	WinsockRuntime& operator=(const WinsockRuntime&) = delete;
	WinsockRuntime(WinsockRuntime&&) = delete;
	WinsockRuntime& operator=(WinsockRuntime&&) = delete;

	// WSAStartup 반환값이다. 0은 성공이며 WSAGetLastError로 대체하지 않는다.
	int GetStartupError() const;

private:
	int _startupError;
};

