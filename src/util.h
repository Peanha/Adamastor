#ifndef UTIL_H
#define UTIL_H

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

[[noreturn]] inline void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::fputs("fatal: ", stderr);
    std::vfprintf(stderr, fmt, args);
    std::fputc('\n', stderr);
    va_end(args);
    std::exit(EXIT_FAILURE);
}

#endif
