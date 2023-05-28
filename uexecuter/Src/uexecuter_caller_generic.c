#include "uexecuter_caller.h"
#if defined(UEXECUTER_CALLER_GENERIC_ENABLED) && UEXECUTER_CALLER_GENERIC_ENABLED
#include <math.h>
#include <string.h>

#define N_MAX_PARAMS_IN_STACK 10
uexecuter_caller_status_t uexecuter_call(void *function_ptr, const uexecuter_caller_param_t *params, size_t n_params,
                                         uexecuter_caller_result_t *result)
{
    result->u64 = -1;
    result->f64 = NAN;
    uint32_t params_in_stack[N_MAX_PARAMS_IN_STACK];
    uint32_t n_params_in_stack = 0;
    for (int i = 0; i < n_params; ++i)
    {
        switch (params[i].type)
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_I32:
        case UEXECUTER_CALLER_TYPE_STR: {
            if (n_params_in_stack >= N_MAX_PARAMS_IN_STACK)
            {
                return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
            }
            params_in_stack[n_params_in_stack++] = params[i].u32;
            break;
        }
        case UEXECUTER_CALLER_TYPE_U64:
        case UEXECUTER_CALLER_TYPE_I64: {
            if (n_params_in_stack + 2 > N_MAX_PARAMS_IN_STACK)
            {
                return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
            }
            memcpy(&params_in_stack[n_params_in_stack], &params[i].u64, sizeof(uint64_t));
            n_params_in_stack += 2;
            break;
        }
#if defined(__SOFT_FP__)
        case UEXECUTER_CALLER_TYPE_F32: {
            if (n_params_in_stack >= N_MAX_PARAMS_IN_STACK)
            {
                return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
            }
            params_in_stack[n_params_in_stack++] = params[i].u32;
            break;
        }
        case UEXECUTER_CALLER_TYPE_F64: {
            if (n_params_in_stack + 2 > N_MAX_PARAMS_IN_STACK)
            {
                return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
            }
            memcpy(&params_in_stack[n_params_in_stack], &params[i].f64, sizeof(double));
            n_params_in_stack += 2;
            break;
        }
#else
        case UEXECUTER_CALLER_TYPE_F32:
        case UEXECUTER_CALLER_TYPE_F64: {
            // floating-point is not supported in hard abi
            return UEXECUTER_CALLER_STATUS_FP_NOT_SUPPORTED;
        }
#endif
        default:
            return UEXECUTER_CALLER_STATUS_INVALID_PARAMETER;
        }
    }
    uint32_t *a = params_in_stack; // use an alias to shorten our code
    switch (n_params_in_stack)
    {
    case 0:
        result->u64 = ((uint64_t(*)())(function_ptr))();
        return UEXECUTER_CALLER_STATUS_OK;
    case 1:
        result->u64 = ((uint64_t(*)(uint32_t))(function_ptr))(a[0]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 2:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t))(function_ptr))(a[0], a[1]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 3:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t))(function_ptr))(a[0], a[1], a[2]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 4:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t))(function_ptr))(a[0], a[1], a[2], a[3]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 5:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))(function_ptr))(a[0], a[1], a[2],
                                                                                                      a[3], a[4]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 6:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))(function_ptr))(
            a[0], a[1], a[2], a[3], a[4], a[5]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 7:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))(
            function_ptr))(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 8:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))(
            function_ptr))(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 9:
        result->u64 = ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                    uint32_t))(function_ptr))(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
        return UEXECUTER_CALLER_STATUS_OK;
    case 10:
        result->u64 =
            ((uint64_t(*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                          uint32_t))(function_ptr))(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
        return UEXECUTER_CALLER_STATUS_OK;
    default:
        return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
    }
}
#endif