#pragma once

#include "keycodes.hpp"

namespace application {

    struct BaseEvent
    {
        mutable bool handled { false };
    };

    template <typename T>
    concept IsEvent = std::is_base_of_v<BaseEvent, T>;

    struct WindowClosedEvent final : public BaseEvent
    {
        explicit constexpr WindowClosedEvent() noexcept = default;
    };

    struct WindowMinimizedEvent final : public BaseEvent
    {
        bool minimized;

        explicit constexpr WindowMinimizedEvent(bool minimized) noexcept
            : minimized(minimized) {}
    };

    struct WindowResizedEvent final : public BaseEvent
    {
        u32 width;
        u32 height;

        explicit constexpr WindowResizedEvent(u32 w, u32 h) noexcept
            : width(w), height(h) {}
    };

    struct KeyEvent : public BaseEvent
    {
        KeyCode keycode;

    protected:
        constexpr KeyEvent(const KeyCode& keycode) noexcept
            : keycode(keycode) {}
    };

    struct KeyPressedEvent final : public KeyEvent
    {
        bool repeat;

        explicit constexpr KeyPressedEvent(KeyCode keycode, bool repeat) noexcept
            : KeyEvent(keycode), repeat(repeat) {}
    };

    struct KeyReleasedEvent final : public KeyEvent
    {
        explicit constexpr KeyReleasedEvent(KeyCode keycode) noexcept
            : KeyEvent(keycode) {}
    };

    struct KeyTypedEvent final : public BaseEvent
    {
        u32 codepoint;

        explicit constexpr KeyTypedEvent(u32 codepoint) noexcept
            : codepoint(codepoint) {}
    };

    struct MouseButtonEvent : public BaseEvent
    {
        MouseButton button;

    protected:
        constexpr MouseButtonEvent(MouseButton button) noexcept
            : button(button) {}
    };

    struct MouseButtonPressedEvent final : public MouseButtonEvent
    {
        explicit constexpr MouseButtonPressedEvent(MouseButton button) noexcept
            : MouseButtonEvent(button) {}
    };

    struct MouseButtonReleasedEvent final : public MouseButtonEvent
    {
        explicit constexpr MouseButtonReleasedEvent(MouseButton button) noexcept
            : MouseButtonEvent(button) {}
    };

    struct MouseEvent : public BaseEvent
    {
        f32 x;
        f32 y;

    protected:
        constexpr MouseEvent(f32 x, f32 y) noexcept
            : x(x), y(y) {}
    };

    struct MouseMovedEvent final : public MouseEvent
    {
        explicit constexpr MouseMovedEvent(f32 x, f32 y) noexcept
            : MouseEvent(x, y) {}
    };

    struct MouseScrolledEvent final : public MouseEvent
    {
        explicit constexpr MouseScrolledEvent(f32 x, f32 y) noexcept
            : MouseEvent(x, y) {}
    };

    template <IsEvent... T>
    using EventVariant = std::variant<T...>;

    using Event = EventVariant<
        WindowClosedEvent,
        WindowMinimizedEvent,
        WindowResizedEvent,
        KeyPressedEvent,
        KeyReleasedEvent,
        KeyTypedEvent,
        MouseButtonPressedEvent,
        MouseButtonReleasedEvent,
        MouseMovedEvent,
        MouseScrolledEvent
    >;

    class EventDispatcher
    {
        friend class Application;
    public:
        EventDispatcher() = default;

        template <IsEvent T, typename Callback>
        void subscribe(Callback&& callback)
        {
            constexpr usize index = event_index<T>::value;
            static_assert(index != std::variant_npos, "error: subscribed type is not an event variant");

            m_observers[index].emplace_back(
                [cb = std::forward<Callback>(callback)](const Event& e) -> bool
                {
                    if (const T* event = std::get_if<T>(&e)) {
                        cb(*event);
                        return event->handled;
                    }

                    return false;
                }
            );
        }

        template <IsEvent T, typename Class>
        void subscribe(Class* instance, bool(Class::*method)(const T&))
        {
            subscribe<T>([instance, method](const T& e) -> bool {
                return (instance->*method)(e);
            });
        }

        void dispatch(const Event& e)
        {
            if (e.valueless_by_exception()) [[unlikely]] return;

            const usize index = e.index();
            auto& listeners = m_observers[index];

            for (auto& handler : listeners) {
                if (handler(e)) break;
            }
        }

    private:
        template <IsEvent T>
        struct event_index
        {
            static constexpr usize value = []() consteval
            {
                usize idx = 0;
                using VariantType = application::Event;

                bool found = [&]<typename... Ts>(std::variant<Ts...>*) {
                    return ((std::is_same_v<T, Ts> ? true : (idx++, false)) || ...);
                }(static_cast<VariantType*>(nullptr));

                return found ? idx : std::variant_npos;
            }();
        };

        #ifdef __cpp_lib_move_only_function
            using EventHandler = std::move_only_function<bool(const Event&)>;
        #else
            using EventHandler = std::function<bool(const Event&)>;
        #endif

    private:
        std::array<std::vector<EventHandler>, std::variant_size_v<Event>> m_observers;
    };

}
