#pragma once
#include <string>
#include <format>
#include <mutex>
#include <stdexcept>
#include "Engine/Platform/WinInclude.h"


namespace KSEngine
{
	class KSLogger {
	public:
		enum class Level {
			Info, Warning, Error, Fatal
		};

		static std::wstring FormatHRESULT(HRESULT hr);

		template<typename... Args>
		static void Log(Level level, const char* file, int line, const char* func, std::wstring_view fmt, Args&&... args)
		{
			std::wstring msg = std::vformat(fmt, std::make_wformat_args(args...));
			LogImpl(level, file, line, func, msg);
		}

		//[[noreturn]]
		static void Fatal(const char* file, int line, const char* func, std::wstring_view msg);

	private:
		static void LogImpl(Level level, const char* file, int line, const char* func, const std::wstring& msg);
		static void WriteCrashLog(const std::wstring& msg);
		static std::mutex s_mutex;
	};
}

// Macros for easy logging with file/line/function
#define LOG_INFO(fmt, ...)    KSEngine::KSLogger::Log(KSEngine::KSLogger::Level::Info,    __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) KSEngine::KSLogger::Log(KSEngine::KSLogger::Level::Warning, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   KSEngine::KSLogger::Log(KSEngine::KSLogger::Level::Error,   __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)   KSEngine::KSLogger::Log(KSEngine::KSLogger::Level::Fatal,   __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_HRINFO(hr) LOG_INFO(L"HRESULT success: {}", KSEngine::KSLogger::FormatHRESULT(hr))
#define LOG_HRWARNING(hr) LOG_WARNING(L"HRESULT warning: {}", KSEngine::KSLogger::FormatHRESULT(hr))
#define LOG_HRERROR(hr) LOG_ERROR(L"HRESULT failure: {}", KSEngine::KSLogger::FormatHRESULT(hr))
