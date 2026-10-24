#include "Core/WinsockRuntime.h"

int main()
{
	const ErrorCode logInitResult = GLogger->Init(LogOutput::Console, LogLevel::Info,
		"logs/backend-server.log");
	if (logInitResult != ErrorCode::EC_SUCCESS)
	{
		LOG_FATAL("서버 로그 초기화에 실패했습니다. 코드=%d (%s)",
			static_cast<int>(logInitResult), GetErrorMessage(logInitResult));
		return 1;
	}

	const WinsockRuntime winsockRuntime;
	const int startupError = winsockRuntime.GetStartupError();
	if (startupError != 0)
	{
		LOG_ERROR("서버 Winsock 초기화 실패: %d", startupError);
		return 1;
	}

	LOG_INFO("서버 템플릿: Winsock 초기화 완료. IOCP 연결 처리는 아직 구현하지 않았습니다.");
	return 0;
}
