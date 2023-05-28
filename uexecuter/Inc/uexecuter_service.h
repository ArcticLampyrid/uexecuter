#pragma once
#include "uexecuter_caller.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        void *ptr;
        uint8_t name_len;
        const char *name;
        uexecuter_caller_type_t result_type;
        uint8_t n_params;
        uexecuter_caller_type_t param_type[32];
    } uexecuter_function_prototype_t;

    typedef struct
    {
        size_t n_function;
        const uexecuter_function_prototype_t *function_prototypes;
    } uexecuter_function_service_info_t;

    const uexecuter_function_prototype_t *uexecuter_service_find_by_name(uexecuter_function_service_info_t *inst,
                                                                         const char *name_begin, const char *name_end);
    const uexecuter_function_prototype_t *uexecuter_service_find_by_address(uexecuter_function_service_info_t *inst,
                                                                            void *ptr);

#define UEXECUTER_DEFINE_SERVICE(name) static const uexecuter_function_prototype_t name[] =
#define UEXECUTER_SERVICE_N_FUNC(name) (sizeof(name) / sizeof(uexecuter_function_prototype_t))

#define UEXECUTER_FUNCTION_PROTOTYPE(_ptr, _name, _result_type, ...)                                                   \
    {                                                                                                                  \
        .ptr = (void *)(_ptr), .name_len = (sizeof(_name) - 1), .name = (_name), .result_type = (_result_type),        \
        .n_params = (sizeof((int[]){0, ##__VA_ARGS__}) / sizeof(int) - 1), .param_type = {                             \
            __VA_ARGS__                                                                                                \
        }                                                                                                              \
    }
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ only part
template <typename TResult, typename... TArgs>
constexpr uexecuter_function_prototype_t uexecuter_function_prototype_auto(TResult (*func)(TArgs... args),
                                                                           const char *name, uint8_t name_len)
{
    uexecuter_function_prototype_t result;
    result.ptr = (void *)func;
    result.name = name;
    result.name_len = name_len;
    result.result_type = uexecuter_caller_type_id_of<TResult>::value;
    int n = 0;
    [[maybe_unused]] int _dummy[] = {(result.param_type[n++] = uexecuter_caller_type_id_of<TArgs>::value, 0)...};
    result.n_params = n;
    return result;
}

#define UEXECUTER_FUNCTION_PROTOTYPE_AUTO(_ptr, _name) uexecuter_function_prototype_auto(_ptr, _name, sizeof(_name) - 1)
#endif