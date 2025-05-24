#include "KSLogger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <comdef.h>
#include "Engine/Platform/WinInclude.h"

std::mutex KSEngine::KSLogger::s_mutex;

namespace
{
	constexpr WORD COLOR_INFO = 7;   // Light gray
	constexpr WORD COLOR_WARNING = 14;  // Yellow
	constexpr WORD COLOR_ERROR = 12;  // Red

	WORD GetColor(KSEngine::KSLogger::Level level)
	{
		switch (level)
		{
		case KSEngine::KSLogger::Level::Info:    return COLOR_INFO;
		case KSEngine::KSLogger::Level::Warning: return COLOR_WARNING;
		case KSEngine::KSLogger::Level::Error:   return COLOR_ERROR;
		case KSEngine::KSLogger::Level::Fatal:   return COLOR_ERROR;
		default: return COLOR_INFO;
		}
	}

	std::wstring LevelToString(KSEngine::KSLogger::Level level)
	{
		switch (level)
		{
		case KSEngine::KSLogger::Level::Info:    return L"INFO";
		case KSEngine::KSLogger::Level::Warning: return L"WARNING";
		case KSEngine::KSLogger::Level::Error:   return L"ERROR";
		case KSEngine::KSLogger::Level::Fatal:   return L"FATAL";
		default: return L"UNKNOWN";
		}
	}

	std::wstring GetTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);
		std::wstringstream ss;
		tm local_tm;
		localtime_s(&local_tm, &t);
		ss << std::put_time(&local_tm, L"%Y-%m-%d %H:%M:%S");
		return ss.str();
	}

	std::wstring ToWide(const char* str)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.data(), len);
		if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
		return wstr;
	}
}

namespace KSEngine
{
	void KSLogger::LogImpl(Level level, const char* file, int line, const char* func, const std::wstring& msg)
	{
		std::lock_guard lock(s_mutex);

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		WORD oldColor = csbi.wAttributes;

		std::wstring wfile = ToWide(file);
		std::wstring wfunc = ToWide(func);

		SetConsoleTextAttribute(hConsole, GetColor(level));
		std::wcout << L"[" << GetTimestamp() << L"] [" << LevelToString(level) << L"] "
			<< L"(" << wfile << L":" << line << L" " << wfunc << L") "
			<< msg << std::endl;
		SetConsoleTextAttribute(hConsole, oldColor);

		if (level == Level::Fatal)
		{
			WriteCrashLog(L"(" + wfile + L":" + std::to_wstring(line) + L" " + wfunc + L") " + msg);
			throw std::runtime_error("Fatal error: " + std::string(msg.begin(), msg.end()));
		}
	}

	void KSLogger::WriteCrashLog(const std::wstring& msg)
	{
		std::wofstream file(L"crash.log", std::ios::app);
		if (file.is_open())
		{
			file << L"[" << GetTimestamp() << L"] [FATAL] " << msg << std::endl;
		}
	}


	std::wstring KSLogger::FormatHRESULT(HRESULT hr)
	{
		std::wstringstream ss;
		ss << L"0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << hr;
		_com_error err(hr);
		ss << L" (" << err.ErrorMessage() << L")";
		return ss.str();
	}

	void KSLogger::Fatal(const char* file, int line, const char* func, std::wstring_view msg)
	{
		LogImpl(Level::Fatal, file, line, func, std::wstring(msg));
		std::terminate();
	}
}