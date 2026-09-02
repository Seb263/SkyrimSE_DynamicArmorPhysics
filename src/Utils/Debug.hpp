#pragma once

#include <source_location>

inline int debugVerboseMode = 1;

class Debug
{
public:
	template <typename... Args>
	static void TraceWithLocation(const char* file, int line, const std::string& fmt, Args&&... args)
	{
		if (debugVerboseMode > 1) {
			spdlog::info("[{}:{}] {}", GetFileName(file), line, fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	static bool ReturnTraceWithLocation(const char* file, int line, const std::string& fmt, Args&&... args)
	{
		TraceWithLocation(file, line, fmt, std::forward<Args>(args)...);
		return false;
	}

	template <typename... Args>
	[[noreturn]] static void ReportAndFailWithLocation(const char* file, int line, const std::string& fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);

		spdlog::log(spdlog::source_loc{ file, line, "" }, spdlog::level::critical, msg);

		const auto plugin = SKSE::PluginDeclaration::GetSingleton();
		const auto name = plugin->GetName();
		const std::string title = fmt::format("{}: Fatal Error", name);

		REX::W32::MessageBoxA(nullptr, msg.c_str(), title.c_str(), MB_ICONERROR);
		REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
	}

private:
	static std::string GetFileName(const char* path)
	{
		std::string strPath(path);
		const auto  pos = strPath.find_last_of("/\\");
		return (pos != std::string::npos) ? strPath.substr(pos + 1) : strPath;
	}
};

// Macros
#define TRACE(fmt, ...)                                                                   \
	do {                                                                                  \
		if (debugVerboseMode > 1) [[unlikely]] {                                          \
			Debug::TraceWithLocation(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__); \
		}                                                                                 \
	} while (0)
#define RETURN_TRACE(fmt, ...) Debug::ReturnTraceWithLocation(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define REPORT_AND_FAIL(fmt, ...) Debug::ReportAndFailWithLocation(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
