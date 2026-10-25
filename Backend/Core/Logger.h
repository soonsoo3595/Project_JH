#pragma once

#include "Core/CoreGlobal.h"
#include "Core/StatusCode.h"

#include <cstdio>
#include <string>
#include <memory>
#include <mutex>

enum class LogLevel
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

enum class LogOutput
{
	Console,
	File,
	Both
};

class Logger final
{
public:
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(Logger&&) = delete;

	// 초기화 실패 시 기존 설정을 유지한다. 기본 상태는 콘솔·Info다.
	StatusCode Init(LogOutput output, LogLevel minimumLevel = LogLevel::Info,
		const std::string& filePath = "logs/backend.log");
	void SetMinimumLevel(LogLevel minimumLevel);

	// UTF-8 메시지를 그대로 기록한다. Fatal도 프로그램을 종료하지 않는다.
	void Log(LogLevel level, const std::string& message, const char* functionName = "");
	// UTF-8 printf 형식이다. 동적 문자열은 Log 또는 "%s"로 전달한다.
	void Write(LogLevel level, const char* functionName, const char* format, ...);

private:
	// 호출자가 _mutex를 보유한 상태에서 사용한다.
	void WriteRecord(LogLevel level, const char* functionName, const std::string& message);

	struct FileCloser
	{
		void operator()(std::FILE* file) const noexcept;
	};

	std::mutex _mutex;
	LogLevel _minimumLevel = LogLevel::Info;
	bool _isConsoleEnabled = true;
	std::unique_ptr<std::FILE, FileCloser> _file;
};

// Release에서는 Trace·Debug의 인자도 평가하지 않는다.
#ifdef _DEBUG
#define LOG_TRACE(...)		(::GLogger->Write(::LogLevel::Trace, __FUNCTION__, __VA_ARGS__))
#define LOG_DEBUG(...)		(::GLogger->Write(::LogLevel::Debug, __FUNCTION__, __VA_ARGS__))
#else
#define LOG_TRACE(...)		((void)0)
#define LOG_DEBUG(...)		((void)0)
#endif

#define LOG_INFO(...)		(::GLogger->Write(::LogLevel::Info, __FUNCTION__, __VA_ARGS__))
#define LOG_WARNING(...)	(::GLogger->Write(::LogLevel::Warning, __FUNCTION__, __VA_ARGS__))
#define LOG_ERROR(...)		(::GLogger->Write(::LogLevel::Error, __FUNCTION__, __VA_ARGS__))
#define LOG_FATAL(...)		(::GLogger->Write(::LogLevel::Fatal, __FUNCTION__, __VA_ARGS__))
