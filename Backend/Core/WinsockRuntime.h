#pragma once

#include "Core/StatusCode.h"

// 소켓을 사용하기 전에 Winsock을 초기화하고 확장 함수를 조회한다. 객체 소멸 시 자동으로 정리한다.
class WinsockRuntime final
{
public:
	WinsockRuntime();
	~WinsockRuntime();

	WinsockRuntime(const WinsockRuntime&) = delete;
	WinsockRuntime& operator=(const WinsockRuntime&) = delete;
	WinsockRuntime(WinsockRuntime&&) = delete;
	WinsockRuntime& operator=(WinsockRuntime&&) = delete;

	// WSA 시작과 확장 함수 조회를 합친 전체 초기화 결과다.
	StatusCode GetSocketInitResult() const noexcept		{ return _socketInitResult; }
	int GetWinsockError() const noexcept				{ return _winsockError; }

private:
	StatusCode _socketInitResult = StatusCode::SC_SUCCESS;
	int _startupResult = 0;
	int _winsockError = 0;
};
