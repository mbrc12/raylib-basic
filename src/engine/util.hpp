#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "raylib.h"

namespace util {

#ifdef NDEBUG
constexpr void dbg(const char *, ...) {}
#else
inline const char *trim_path(const char *file) {
    const char *p = strstr(file, "src/");
    if (p) return p + 4;
    const char *s = file;
    for (const char *c = file; *c; ++c) {
        if (*c == '/' || *c == '\\') s = c + 1;
    }
    return s;
}

inline void dbg_impl(const char *file, int line, const char *fmt, ...) {
    char buf[256];
    int n = snprintf(buf, sizeof(buf), "%s:%d: ", trim_path(file), line);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf + n, sizeof(buf) - n, fmt, args);
    va_end(args);
    TraceLog(LOG_INFO, "%s", buf);
}

#define dbg(fmt, ...) util::dbg_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

} // namespace util
