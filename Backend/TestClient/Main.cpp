#include "Core/WinsockRuntime.h"

int main()
{
	const StatusCode logInitResult = GLogger->Init(LogOutput::Console, LogLevel::Info,
		"logs/backend-test-client.log");
	if (logInitResult != StatusCode::SC_SUCCESS)
	{
		LOG_FATAL("테스트 클라이언트 로그 초기화에 실패했습니다. 코드=%d (%s)",
			static_cast<int>(logInitResult), GetStatusMessage(logInitResult));
		return 1;
	}

	const WinsockRuntime winsockRuntime;
	const StatusCode networkInitResult = winsockRuntime.GetSocketInitResult();
	if (networkInitResult != StatusCode::SC_SUCCESS)
	{
		LOG_ERROR("테스트 클라이언트 네트워크 초기화 실패: %s (Winsock 오류: %d)", GetStatusMessage(networkInitResult), winsockRuntime.GetWinsockError());
		return 1;
	}

	LOG_INFO("테스트 클라이언트 템플릿: Winsock·확장 함수 초기화 완료. 서버 연결은 아직 구현하지 않았습니다.");
	return 0;
}
