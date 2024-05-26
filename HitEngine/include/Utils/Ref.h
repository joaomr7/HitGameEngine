#pragma once

#include<memory>

namespace hit
{
    // maybe implement my own ref_count system
    template <typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    constexpr Ref<T> create_ref(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    constexpr Ref<T> create_wraper_ref(Args&&... args)
    {
        return std::shared_ptr<T>(new T(std::forward<Args>(args)...), [](T* ptr) { });
    }

    template<typename U, typename T> 
    constexpr Ref<U> cast_ref(const Ref<T> t)
    {
        return std::static_pointer_cast<U>(t);
    }

    template <typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    constexpr Scope<T> create_scope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}