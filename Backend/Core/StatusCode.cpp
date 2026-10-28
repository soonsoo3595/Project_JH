#include "Core/StatusCode.h"

const char* GetStatusMessage(StatusCode code) noexcept
{
	switch (code)
	{
	case StatusCode::SC_SUCCESS: return "성공";

	// 로그
	case StatusCode::SC_LOG_INVALID_LEVEL: return "잘못된 로그 레벨";
	case StatusCode::SC_LOG_INVALID_OUTPUT: return "잘못된 로그 출력 대상";
	case StatusCode::SC_LOG_EMPTY_FILE_PATH: return "빈 로그 파일 경로";
	case StatusCode::SC_LOG_DIRECTORY_CREATION_FAILED: return "로그 디렉터리 생성 실패";
	case StatusCode::SC_LOG_FILE_OPEN_FAILED: return "로그 파일 열기 실패";

	// 네트워크
	case StatusCode::SC_NET_INVALID_ADDRESS: return "잘못된 IPv4 주소";
	case StatusCode::SC_NET_INVALID_SOCKET: return "유효하지 않은 소켓";
	case StatusCode::SC_NET_SOCKET_ALREADY_OPEN: return "소켓이 이미 생성되어 있음";
	case StatusCode::SC_NET_SOCKET_CREATE_FAILED: return "TCP 소켓 생성 실패";
	case StatusCode::SC_NET_SOCKET_OPTION_FAILED: return "소켓 옵션 설정 실패";
	case StatusCode::SC_NET_BIND_FAILED: return "소켓 주소 바인딩 실패";
	case StatusCode::SC_NET_LISTEN_FAILED: return "소켓 연결 대기 설정 실패";
	case StatusCode::SC_NET_SOCKET_CLOSE_FAILED: return "소켓 종료 실패";
	case StatusCode::SC_NET_LOCAL_ADDRESS_FAILED: return "소켓 로컬 주소 조회 실패";
	case StatusCode::SC_NET_INVALID_ARGUMENT: return "잘못된 네트워크 함수 인수";

	// IOCP
	case StatusCode::SC_IOCP_NOT_INITIALIZED: return "IOCP가 초기화되지 않음";
	case StatusCode::SC_IOCP_ALREADY_INITIALIZED: return "IOCP가 이미 초기화되어 있음";
	case StatusCode::SC_IOCP_CREATE_FAILED: return "IOCP 생성 실패";
	case StatusCode::SC_IOCP_REGISTER_FAILED: return "IOCP 소켓 등록 실패";
	case StatusCode::SC_IOCP_WAIT_TIMEOUT: return "IOCP 완료 대기 시간 초과";
	case StatusCode::SC_IOCP_WAIT_FAILED: return "IOCP 완료 대기 실패";
	case StatusCode::SC_IOCP_IO_FAILED: return "완료된 비동기 I/O 작업 실패";
	case StatusCode::SC_IOCP_POST_FAILED: return "IOCP 수동 알림 등록 실패";
	case StatusCode::SC_IOCP_CLOSE_FAILED: return "IOCP 핸들 해제 실패";
	case StatusCode::SC_IOCP_INVALID_OBJECT: return "유효하지 않은 IOCP 객체";

	default: return "정의되지 않은 상태 코드";
	}
}
