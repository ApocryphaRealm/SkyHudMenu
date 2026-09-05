#pragma once

#include "PCH.h"

namespace SKSE::log
{
	using level = spdlog::level::level_enum;

	inline void set_level(level a_log_level, level a_flush_level)
	{
		spdlog::default_logger()->set_level(a_log_level);
		spdlog::default_logger()->flush_on(a_flush_level);
	}

	inline bool init(const std::string_view& a_log_name)
	{
		if (!log_directory())
		{
			return false;
		}

		std::filesystem::path path = *log_directory() / a_log_name;
		path += ".log";
		std::shared_ptr<spdlog::logger> log = spdlog::basic_logger_mt("global log", path.string(), true);

		spdlog::set_default_logger(std::move(log));

		set_level(level::info, level::info);

		spdlog::set_pattern("%D - %H:%M:%S.%f [%^%l%$] %v");

		return true;
	}

	inline void flush()
	{
		spdlog::default_logger()->flush();
	}

	template <class... Args>
	void at_level(spdlog::level::level_enum a_level, fmt::format_string<Args...> a_fmt, Args&&... args)
	{
		switch (a_level) {
		case level::trace:
			trace<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		case level::debug:
			debug<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		case level::info:
			info<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		case level::warn:
			warn<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		case level::err:
			error<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		case level::critical:
			critical<Args...>(a_fmt, std::forward<Args&&>(args)...);
			break;
		}
	}

	// Writes a short header saying which log level is active and exactly what that level does
	// and does not capture, so any log file is self-describing - including one sent in by
	// someone else, where the INI that produced it cannot be checked.
	//
	// Emitted through at_level() at whatever level is currently active, which is by definition
	// at or above the threshold, so it appears no matter what uLogLevel is set to.
	inline void describe_level(const std::string& a_iniFileName)
	{
		const level active = spdlog::default_logger()->level();

		struct Entry
		{
			level       value;
			int         iniValue;
			const char* name;
		};

		// Ordered most verbose to least, matching the uLogLevel numbering in the INI.
		constexpr Entry kEntries[] = {
			{ level::trace,    0, "trace"    },
			{ level::debug,    1, "debug"    },
			{ level::info,     2, "info"     },
			{ level::warn,     3, "warning"  },
			{ level::err,      4, "error"    },
			{ level::critical, 5, "critical" },
		};

		std::string capturing;
		std::string suppressed;
		int         activeIniValue = 2;
		const char* activeName     = "info";

		for (const Entry& entry : kEntries)
		{
			std::string& target = entry.value >= active ? capturing : suppressed;

			if (!target.empty())
			{
				target += ", ";
			}

			target += entry.name;

			if (entry.value == active)
			{
				activeIniValue = entry.iniValue;
				activeName     = entry.name;
			}
		}

		spdlog::default_logger()->log(active, "Log level {} ({}). Capturing: {}.", activeIniValue, activeName, capturing);

		if (suppressed.empty())
		{
			spdlog::default_logger()->log(active, "Nothing is suppressed - this is the most comprehensive level.");
		}
		else
		{
			spdlog::default_logger()->log(active, "Suppressed at this level: {}. Set uLogLevel=0 in {} to capture everything.",
					 suppressed, a_iniFileName);
		}
	}
}

namespace GFxLogger
{
	struct info;
	struct error;

	bool RegisterStaticFunctions(RE::GFxMovieView* a_view);
}

namespace logger = SKSE::log;