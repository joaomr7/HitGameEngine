#pragma once

#include "Core/Types.h"
#include "Core/Module.h"
#include "Event.h"

#include <string>
#include <string_view>

namespace hit
{
    // do not include Window.h here, because it need to link just in engine
    // to access window attributes, use the platform api
    class Window;
    struct WindowSpecification;

    // engine representation of external dll's(or platform specific)
    struct ExternalPackage;
    using PackageFunction = void*;

    class Platform : public Module
    {
    public:
        static Window* create_window(const WindowSpecification& specification, const EventCallback& callback);
        static void destroy_window(Window* window);

        static bool is_window_open(Window* window);
        static void close_window(Window* window);

        static ui16 get_window_width(Window* window);
        static ui16 get_window_height(Window* window);

        static const std::string& get_window_title(Window* window);

        static const Window* get_main_window();

        static void wait_for_valid_window_size(Window* window);

        // used to load dll's(or platform specific)
        static ExternalPackage* load_external_package(std::string_view package_path);
        static bool unload_external_package(ExternalPackage* package);
        static PackageFunction get_package_function(ExternalPackage* package, std::string_view function_name);

    protected:
        bool initialize() override;
        void shutdown() override;
        bool execute() override;

    private:
        static Window* s_main_window;
        static ui16 s_window_count;
    };
}