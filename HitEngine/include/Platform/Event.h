#pragma once

#include "Core/Types.h"

#include <functional>

namespace hit
{
    struct Event
    {
        enum Type
        {
            None,
            KeyboardKeyPressed, KeyboardKeyReleased, KeyboardKeyHold,
            MouseButtonPressed, MouseButtonReleased, MouseScrolled, MouseMoved,
            WindowClose, WindowResize
        };

        bool handled;

        virtual inline Type get_event_type() { return None; }
        static inline Type get_static_event_type() { return None; }
    };

    using EventCallback = std::function<void(Event&)>;
    #define bind_event_function(fun, owner) std::bind(&fun, owner, std::placeholders::_1)

    class EventHandler
    {
    public:
        EventHandler(Event& event) : m_event(event) { }

        template <typename T, typename Function>
        bool handle(const Function& fun)
        {
            if(m_event.get_event_type() == T::get_static_event_type())
            {
                m_event.handled |= fun(static_cast<T&>(m_event));
                return true;
            }

            return false;
        };

    private:
        Event& m_event;
    };

    #define register_event(event_type)                                                      \
        inline Event::Type get_event_type() override { return Event::event_type; }          \
        static inline Event::Type get_static_event_type() { return Event::event_type; }

    struct KeyboardKeyPressedEvent : public Event
    {
        constexpr KeyboardKeyPressedEvent(i32 key) : key(key) { }
        register_event(KeyboardKeyPressed);

        i32 key;
    };
    
    struct KeyboardKeyReleasedEvent : public Event
    {
        constexpr KeyboardKeyReleasedEvent(i32 key) : key(key) { }
        register_event(KeyboardKeyReleased);

        i32 key;
    };

    struct KeyboardKeyHoldEvent : public Event
    {
        constexpr KeyboardKeyHoldEvent(i32 key) : key(key) { }
        register_event(KeyboardKeyHold);

        i32 key;
    };

    struct MouseButtonPressedEvent : public Event
    {
        constexpr MouseButtonPressedEvent(i32 button) : button(button) { }
        register_event(MouseButtonPressed);

        i32 button;
    };

    struct MouseButtonReleasedEvent : public Event
    {
        constexpr MouseButtonReleasedEvent(i32 button) : button(button) { }
        register_event(MouseButtonReleased);

        i32 button;
    };

    struct MouseScrolledEvent : public Event
    {
        constexpr MouseScrolledEvent(f32 offset_x, f32 offset_y) : offset_x(offset_x), offset_y(offset_y) { }
        register_event(MouseScrolled);

        f32 offset_x;
        f32 offset_y;
    };

    struct MouseMovedEvent : public Event
    {
        constexpr MouseMovedEvent(f32 mouse_x, f32 mouse_y) : mouse_x(mouse_x), mouse_y(mouse_y) { }
        register_event(MouseMoved);

        f32 mouse_x;
        f32 mouse_y;
    };

    struct WindowCloseEvent : public Event
    {
        constexpr WindowCloseEvent() = default;
        register_event(WindowClose);
    };

    struct WindowResizeEvent : public Event
    {
        constexpr WindowResizeEvent(ui16 width, ui16 height) : width(width), height(height) {}
        register_event(WindowResize);

        ui16 width;
        ui16 height;
    };

    #undef register_event
}