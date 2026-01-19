#pragma once

namespace application {

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    #if __STDCPP_FLOAT32_T__ == 1
        #include <stdfloat>

        using f32 = std::float32_t;
        using f64 = std::float64_t;
    #else
        using f32 = float;
        using f64 = double;
    #endif

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

    template<typename T>
    concept UnsignedInt = 
        std::same_as<T, u8>  || std::same_as<T, u16> || 
        std::same_as<T, u32> || std::same_as<T, u64> || 
        std::same_as<T, usize>;

    template<typename T>
    concept SignedInt = 
        std::same_as<T, i8>  || std::same_as<T, i16> || 
        std::same_as<T, i32> || std::same_as<T, i64> || 
        std::same_as<T, isize>;

    template<typename T>
    concept Float = std::same_as<T, f32> || std::same_as<T, f64>;

    // template<typename T>
    // concept Primitive = UnsignedInt<T> || SignedInt<T> || Float<T>;

    static_assert(sizeof(u8)  == 1, "u8 must be 1 byte");
    static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
    static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
    static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");

    static_assert(sizeof(i8)  == 1, "i8 must be 1 byte");
    static_assert(sizeof(i16) == 2, "i16 must be 2 bytes");
    static_assert(sizeof(i32) == 4, "i32 must be 4 bytes");
    static_assert(sizeof(i64) == 8, "i64 must be 8 bytes");

    static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
    static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

    static_assert(sizeof(usize) == sizeof(void*), "usize must be pointer size");
    static_assert(sizeof(isize) == sizeof(void*), "isize must be pointer size");

}
