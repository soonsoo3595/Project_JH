#pragma once

// EC_ 접두사와 대문자 스네이크 표기를 사용한다. 성공은 0, 오류는 종류별 100개 단위로 묶는다.
// 100~199: 로거. 새 종류의 구간은 필요할 때 추가하며 확정된 번호는 재사용하지 않는다.
enum class ErrorCode : int
{
	EC_SUCCESS = 0,
	EC_LOG_INVALID_LEVEL = 100,
	EC_LOG_INVALID_OUTPUT = 101,
	EC_LOG_EMPTY_FILE_PATH = 102,
	EC_LOG_DIRECTORY_CREATION_FAILED = 103,
	EC_LOG_FILE_OPEN_FAILED = 104
};

const char* GetErrorMessage(ErrorCode code) noexcept;
