#include "Core/ErrorCode.h"

const char* GetErrorMessage(ErrorCode code) noexcept
{
	switch (code)
	{
	case ErrorCode::EC_SUCCESS: return "성공";
	case ErrorCode::EC_LOG_INVALID_LEVEL: return "잘못된 로그 레벨";
	case ErrorCode::EC_LOG_INVALID_OUTPUT: return "잘못된 로그 출력 대상";
	case ErrorCode::EC_LOG_EMPTY_FILE_PATH: return "빈 로그 파일 경로";
	case ErrorCode::EC_LOG_DIRECTORY_CREATION_FAILED: return "로그 디렉터리 생성 실패";
	case ErrorCode::EC_LOG_FILE_OPEN_FAILED: return "로그 파일 열기 실패";
	default: return "정의되지 않은 오류 코드";
	}
}
