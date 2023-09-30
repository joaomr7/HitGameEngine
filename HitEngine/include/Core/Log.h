#pragma once

#include <format>
#include <string>
#include <string_view>

#ifdef HIT_RELEASE
#define hit_trace(fmt, ...)
#define hit_info(fmt, ...)
#define hit_warning(fmt, ...)
#define hit_error(fmt, ...)
#define hit_fatal(fmt, ...)

#define hit_trace_if(condition, fmt, ...)
#define hit_info_if(condition, fmt, ...)
#define hit_warning_if(condition, fmt, ...)
#define hit_error_if(condition, fmt, ...)
#define hit_fatal_if(condition, fmt, ...)
#else
#define hit_trace(fmt, ...)     hit::Log::log_message(hit::Log::LogLevel::Trace,   fmt, __VA_ARGS__)
#define hit_info(fmt, ...)      hit::Log::log_message(hit::Log::LogLevel::Info,    fmt, __VA_ARGS__)
#define hit_warning(fmt, ...)   hit::Log::log_message(hit::Log::LogLevel::Warning, fmt, __VA_ARGS__)
#define hit_error(fmt, ...)     hit::Log::log_message(hit::Log::LogLevel::Error,   fmt, __VA_ARGS__)
#define hit_fatal(fmt, ...)     hit::Log::log_message(hit::Log::LogLevel::Fatal,   fmt, __VA_ARGS__)

#define hit_trace_if(condition, fmt, ...)       { if((condition)) { hit_trace(fmt,   __VA_ARGS__); } }
#define hit_info_if(condition, fmt, ...)        { if((condition)) { hit_info(fmt,    __VA_ARGS__); } }
#define hit_warning_if(condition, fmt, ...)     { if((condition)) { hit_warning(fmt, __VA_ARGS__); } }
#define hit_error_if(condition, fmt, ...)       { if((condition)) { hit_error(fmt,   __VA_ARGS__); } }
#define hit_fatal_if(condition, fmt, ...)       { if((condition)) { hit_fatal(fmt,   __VA_ARGS__); } }
#endif

namespace hit
{
    namespace Log
    {
        enum class LogLevel
        {
            Trace,
            Info,
            Warning,
            Error,
            Fatal
        };

        bool initialize_log_system();
        void shutdown_log_system();
        void process_log(LogLevel level, std::string_view message);

        template <typename... Args> 
        constexpr std::string log_formatter(std::string_view raw_format, Args&&... args)
        {
            return std::vformat(raw_format, std::make_format_args(args...));
        }

        template <typename... Args>
        void log_message(LogLevel level, std::string_view raw_message, Args&&... args)
        {
            std::string formatted_message = log_formatter(raw_message, args...);
            process_log(level, formatted_message);
        }
    }
}