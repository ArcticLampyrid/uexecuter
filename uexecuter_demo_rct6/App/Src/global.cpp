#define _GNU_SOURCE
#include "global.h"
#include "usart.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
extern "C" void debug_log(const char *format, ...)
{
    char *buf;
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(&buf, format, ap);
    va_end(ap);
    if (retval != -1)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)buf, retval, HAL_MAX_DELAY);
    }
    free(buf);
}