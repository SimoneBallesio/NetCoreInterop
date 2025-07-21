#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;
typedef float f32;
typedef double f64;
typedef int b32;
typedef bool b8;

#if defined(__clang__) || defined(__GNUC__)
#define STATIC_ASSERT _Static_assert

#else
#define STATIC_ASSERT static_assert
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define INTEROP_PLATFORM_WINDOWS 1

#ifndef _WIN64
#error "64-bit is required on Windows!"

#endif

#elif defined(__linux__) || defined(__gnu_linux__)
#define INTEROP_PLATFORM_LINUX 1

#if defined(__ANDROID__)
#define INTEROP_PLATFORM_ANDROID 1
#endif

#elif __APPLE__
#define INTEROP_PLATFORM_APPLE 1

#else
#error "Unsupported platform"
#endif

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
#define INTEROP_PLATFORM_UNIX 1

#elif defined(_POSIX_VERSION)
#define INTEROP_PLATFORM_POSIX 1
#endif

#ifdef INTEROP_EXPORT
#ifdef _MSC_VER
#define INTEROP_API __declspec(dllexport)
#define INTEROP_C_API extern "C" __declspec(dllexport)

#else
#define INTEROP_API __attribute__((visibility("default")))
#define INTEROP_C_API extern "C" __attribute__((visibility("default")))
#endif

#else

#ifdef _MSC_VER
#define INTEROP_API __declspec(dllimport)
#define INTEROP_C_API extern "C" __declspec(dllimport)

#else
#define INTEROP_API
#define INTEROP_C_API
#endif
#endif

#ifdef _DEBUG
#define INTEROP_DEBUG

#else
#define INTEROP_RELEASE
#endif

#if defined(__clang__) || defined(__gcc__)
#define INTEROP_INLINE __attribute__((always_inline)) inline
#define INTEROP_NOINLINE __attribute__((noinline))

#elif defined(_MSC_VER)
#define INTEROP_INLINE __forceinline
#define INTEROP_NOINLINE __declspec(noinline)

#else
#define INTEROP_INLINE inline
#define INTEROP_NOINLINE

#endif