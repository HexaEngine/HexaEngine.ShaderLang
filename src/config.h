#ifndef CONFIG_H
#define CONFIG_H

#include <cassert>
#include <iostream>

#if defined _WIN32 || defined __CYGWIN__
#ifdef CIMGUI_NO_EXPORT
#define API
#else
#define API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define API  __attribute__((__visibility__("default")))
#else
#define API
#endif
#endif

#if defined __cplusplus
#define EXTERN extern "C"
#else
#include <stdarg.h>
#include <stdbool.h>
#define EXTERN extern
#endif

#define HXSL_API EXTERN API

static bool EnableAsserts = true;
static bool EnableErrorOutput = true;

#define HXSL_ASSERT(expr, message) \
    do { \
        bool _result = (expr);  \
        if (!_result && EnableAsserts) { \
            std::cerr << "ASSERTION FAILED: " << message << std::endl; \
            assert(false && message); \
        } \
    } while (0);

#define MAX_LOG_LENGTH 512

#define ILVersionString "1.0.0.0"

#define ILVersion ((1 << 24) | (0 << 16) | (0 << 8) | 0);

typedef unsigned int uint;

#define DEBUG

#define ALLOW_PARTIAL_ANALYSIS

constexpr auto QUALIFIER_SEP = '.';

#endif