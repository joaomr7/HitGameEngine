#include "Core/Log.h"

#include <iostream>
#include <ctime>

struct Console
{
    enum Color
    {
        ColorWhite = 0,
        ColorGreen = 1,
        ColorYellow = 2,
        ColorRed = 3,
        ColorCount
    };

    Console() = default;
    virtual ~Console() = default;

    virtual void set_color(Color color) = 0;
    virtual void reset_color() = 0;
};

#ifdef HIT_PLATFORM_WINDOWS
#include <windows.h>

struct WindowsConsole : public Console
{
    HANDLE console_handle;
    int default_attributes;
    int colors[Console::ColorCount] =
    {
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,    // White
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,                // Green
        FOREGROUND_RED | FOREGROUND_GREEN,                      // Yellow
        FOREGROUND_RED | FOREGROUND_INTENSITY                   // Red
    };

    WindowsConsole()
    {
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO console_info;
        GetConsoleScreenBufferInfo(console_handle, &console_info);

        default_attributes = console_info.wAttributes;
    }

    inline void set_attribute(int attribute)
    {
        SetConsoleTextAttribute(console_handle, attribute);
    }

    void set_color(Color color) override
    {
        int color_attribute = colors[color];
        set_attribute(color_attribute);
    }

    void reset_color() override
    {
        set_attribute(default_attributes);
    }
};
#else
#error Invalid Platform
#endif

namespace hit::Log
{
    static Console* s_console = nullptr;

    bool initialize_log_system()
    {
#ifdef HIT_PLATFORM_WINDOWS
        s_console = new WindowsConsole();
#endif

        return true;
    }

    void shutdown_log_system()
    {
        delete s_console;
        s_console = nullptr;
    }

    void process_log(LogLevel level, std::string_view message)
    {
        std::time_t current_time = std::time(0);
        std::tm time_info;
        localtime_s(&time_info, &current_time);

        const char* level_message;
        Console::Color color = Console::ColorWhite;

        switch (level)
        {
            case LogLevel::Trace:   
            {
                level_message = (const char*)"[TRACE]";
                color = Console::ColorWhite;
                break;
            }

            case LogLevel::Info:    
            {
                level_message = (const char*)"[INFO]"; 
                color = Console::ColorGreen;
                break;
            }

            case LogLevel::Warning:
            {
                level_message = (const char*)"[WARNING]"; 
                color = Console::ColorYellow;
                break;
            }

            case LogLevel::Error:
            {
                level_message = (const char*)"[ERROR]"; 
                color = Console::ColorRed;
                break;
            }

            case LogLevel::Fatal:
            {
                level_message = (const char*)"[FATAL]"; 
                color = Console::ColorRed;
                break;
            }
        }   

        s_console->set_color(color);

        std::cout << std::format(
            "{} {:02}:{:02}:{:02}:  {}\n", 
            level_message, 
            time_info.tm_hour, 
            time_info.tm_min, 
            time_info.tm_sec, 
            message);

        s_console->reset_color();
    }
}