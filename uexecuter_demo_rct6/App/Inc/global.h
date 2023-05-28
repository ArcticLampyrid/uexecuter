#pragma once
#ifdef __cplusplus
// C++-only 部分
extern "C"
{
#endif
    // 兼容C的部分
    void debug_log(const char *format, ...);
#ifdef __cplusplus
}
#endif