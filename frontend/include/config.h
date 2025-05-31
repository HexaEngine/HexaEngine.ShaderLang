#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>
#include <stdio.h>

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

#ifndef HXSL_ENABLE_CAPI
#ifdef __cplusplus
#define HXSL_ENABLE_CAPI 0
#else
#define HXSL_ENABLE_CAPI 1
#endif
#endif

#ifdef __cplusplus
#define C_API_BEGIN extern "C" {
#define C_API_END   }
#include <cstdint>
#else
#define C_API_BEGIN
#define C_API_END
#include <stdint.h>
#include <stddef.h>
#endif

extern bool EnableAsserts;
extern bool EnableErrorOutput;

#define HXSL_ASSERT(expr, message) \
    do { \
        bool _result = (expr);  \
        if (!_result && EnableAsserts) { \
            fprintf(stderr, "ASSERTION FAILED: %s\n", message); \
            assert(false && message); \
        } \
    } while (0);

#ifndef MAX_LOG_LENGTH
#define MAX_LOG_LENGTH 512
#endif

#define ILVersionString "1.0.0.0"

#define ILVersion ((1 << 24) | (0 << 16) | (0 << 8) | 0)

#define ALLOW_PARTIAL_ANALYSIS

#ifndef QUALIFIER_SEP
#define QUALIFIER_SEP '.'
#endif

#define HXSL_ASSERT_DEPRECATION \
    HXSL_ASSERT(false, "Use of deprecated method.")

#ifndef HXSL_LOCALE_PATH
#define HXSL_LOCALE_PATH "./locales"
#endif

#ifndef HXSL_DEBUG
#define HXSL_DEBUG 0
#endif

#ifdef __cplusplus
//#define dynamic_cast static_assert(false, "dynamic_cast is forbidden!")
#endif

#endif // CONFIG_H