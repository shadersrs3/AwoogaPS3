#include <cstdarg>

#include <Common/Logger.h>

void logOutput(FILE *out, const char *code, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    switch (*code) {
    case 'W':
        fprintf(out, "\033[93m[%s]: ", code);
        break;
    case 'E':
        fprintf(out, "\033[1;41;37m[%s]: ", code);
        break;
    case 'I':
        fprintf(out, "\033[1m[%s]: ", code);
        break;
    }

    vfprintf(out, fmt, ap);
    fprintf(out, "\033[0m\n");
    va_end(ap);
}
