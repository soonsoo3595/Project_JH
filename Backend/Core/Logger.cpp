#include "Core/Logger.h"

#include <windows.h>
#include <share.h>

#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <system_error>

// 가변 인수 목록은 포맷 도중 조기 반환되거나 스택이 풀릴 때도 정리한다.
class LogArgumentScope final
{
public:
	explicit LogArgumentScope(std::va_list& arguments) noexcept
		: _arguments(arguments)
	{
	}

	~LogArgumentScope()
	{
		va_end(_arguments);
	}

	LogArgumentScope(const LogArgumentScope&) = delete;
	LogArgumentScope& operator=(const LogArgumentScope&) = delete;

private:
	std::va_list& _arguments;
};

static const char* GetLevelName(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Trace: return "TRACE";
	case LogLevel::Debug: return "DEBUG";
	case LogLevel::Info: return "INFO";
	case LogLevel::Warning: return "WARNING";
	case LogLevel::Error: return "ERROR";
	case LogLevel::Fatal: return "FATAL";
	default: return nullptr;
	}
}

static bool FormatLogMessage(const char* format, std::va_list arguments, std::string& message)
{
	std::va_list sizeArguments;
	va_copy(sizeArguments, arguments);
	const int length = std::vsnprintf(nullptr, 0, format, sizeArguments);
	va_end(sizeArguments);
	if (length < 0)
	{
		return false;
	}
	message.resize(static_cast<std::size_t>(length) + 1);
	const int written = std::vsnprintf(message.data(), message.size(), format, arguments);
	if (written != length)
	{
		return false;
	}
	message.resize(static_cast<std::size_t>(length));
	return true;
}

static std::string MakeRecord(LogLevel level, std::string_view functionName, std::string_view message)
{
	SYSTEMTIME time{};
	GetLocalTime(&time);
	std::ostringstream stream;
	stream.imbue(std::locale::classic());
	stream << std::setfill('0') << '['
		<< std::setw(4) << time.wYear << '-'
		<< std::setw(2) << time.wMonth << '-'
		<< std::setw(2) << time.wDay << ' '
		<< std::setw(2) << time.wHour << ':'
		<< std::setw(2) << time.wMinute << ':'
		<< std::setw(2) << time.wSecond << '.'
		<< std::setw(3) << time.wMilliseconds << "] ["
		<< GetLevelName(level) << "] ";
	if (functionName.empty() == false)
	{
		stream << '[' << functionName << "] ";
	}
	stream << message << "\r\n";
	return stream.str();
}

static void WriteConsoleRecord(LogLevel level, const std::string& record)
{
	const DWORD streamId = level >= LogLevel::Error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE;
	const HANDLE handle = GetStdHandle(streamId);
	if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD mode = 0;
	if (GetConsoleMode(handle, &mode) != 0)
	{
		// 실제 콘솔에서만 Windows UTF-16 경계로 변환한다. 콘솔 코드 페이지는 변경하지 않는다.
		const int byteCount = static_cast<int>(record.size());
		const int characterCount = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			record.data(), byteCount, nullptr, 0);
		if (characterCount == 0)
		{
			return;
		}
		std::wstring wideRecord(static_cast<std::size_t>(characterCount), L'\0');
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, record.data(), byteCount,
			wideRecord.data(), characterCount) != characterCount)
		{
			return;
		}
		std::size_t offset = 0;
		while (offset < wideRecord.size())
		{
			DWORD written = 0;
			if (WriteConsoleW(handle, wideRecord.data() + offset,
				static_cast<DWORD>(wideRecord.size() - offset), &written, nullptr) == 0)
			{
				return;
			}
			if (written == 0)
			{
				return;
			}
			offset += written;
		}
		return;
	}

	// 파일·파이프 리다이렉션에는 UTF-8 바이트를 유지하고 부분 쓰기도 처리한다.
	std::size_t offset = 0;
	while (offset < record.size())
	{
		DWORD written = 0;
		if (WriteFile(handle, record.data() + offset,
			static_cast<DWORD>(record.size() - offset), &written, nullptr) == 0)
		{
			return;
		}
		if (written == 0)
		{
			return;
		}
		offset += written;
	}
	return;
}

void Logger::FileCloser::operator()(std::FILE* file) const noexcept
{
	if (file != nullptr)
	{
		std::fclose(file);
	}
}

ErrorCode Logger::Init(LogOutput output, LogLevel minimumLevel,
	const std::filesystem::path& filePath)
{
	if (output != LogOutput::Console && output != LogOutput::File && output != LogOutput::Both)
	{
		return ErrorCode::EC_LOG_INVALID_OUTPUT;
	}
	if (GetLevelName(minimumLevel) == nullptr)
	{
		return ErrorCode::EC_LOG_INVALID_LEVEL;
	}
	std::unique_ptr<std::FILE, FileCloser> newFile;
	if (output != LogOutput::Console)
	{
		if (filePath.empty())
		{
			return ErrorCode::EC_LOG_EMPTY_FILE_PATH;
		}
		std::error_code error;
		const std::filesystem::path parentPath = filePath.parent_path();
		if (parentPath.empty() == false)
		{
			std::filesystem::create_directories(parentPath, error);
			if (error.value() != 0)
			{
				return ErrorCode::EC_LOG_DIRECTORY_CREATION_FAILED;
			}
		}
		newFile.reset(_wfsopen(filePath.c_str(), L"ab", _SH_DENYNO));
		if (newFile == nullptr)
		{
			return ErrorCode::EC_LOG_FILE_OPEN_FAILED;
		}
	}
	const std::lock_guard<std::mutex> lock(_mutex);
	_file.swap(newFile);
	_minimumLevel = minimumLevel;
	_isConsoleEnabled = output != LogOutput::File;
	return ErrorCode::EC_SUCCESS;
}

void Logger::SetMinimumLevel(LogLevel minimumLevel)
{
	if (GetLevelName(minimumLevel) == nullptr)
	{
		return;
	}
	const std::lock_guard<std::mutex> lock(_mutex);
	_minimumLevel = minimumLevel;
}

void Logger::Log(LogLevel level, std::string_view message, std::string_view functionName)
{
	if (GetLevelName(level) == nullptr)
	{
		return;
	}
	const std::lock_guard<std::mutex> lock(_mutex);
	if (level < _minimumLevel)
	{
		return;
	}
	WriteRecord(level, functionName, message);
}

void Logger::Write(LogLevel level, std::string_view functionName, const char* format, ...)
{
	if (GetLevelName(level) == nullptr || format == nullptr)
	{
		return;
	}
	const std::lock_guard<std::mutex> lock(_mutex);
	if (level < _minimumLevel)
	{
		return;
	}
	std::string message;
	std::va_list arguments;
	va_start(arguments, format);
	const LogArgumentScope argumentScope(arguments);
	if (FormatLogMessage(format, arguments, message) == false)
	{
		return;
	}
	WriteRecord(level, functionName, message);
}

void Logger::WriteRecord(LogLevel level, std::string_view functionName, std::string_view message)
{
	const std::string record = MakeRecord(level, functionName, message);
	if (record.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		return;
	}
	if (_isConsoleEnabled)
	{
		WriteConsoleRecord(level, record);
	}
	// 콘솔 출력 실패와 관계없이 파일 기록도 시도한다.
	if (_file != nullptr)
	{
		const std::size_t written = std::fwrite(record.data(), 1, record.size(), _file.get());
		const int flushResult = std::fflush(_file.get());
		if (written != record.size() || flushResult != 0)
		{
			WriteConsoleRecord(LogLevel::Error, "[LOGGER] 로그 파일 기록에 실패했습니다.\r\n");
		}
	}
}
