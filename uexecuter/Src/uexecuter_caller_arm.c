#include "uexecuter_caller.h"
#if defined(UEXECUTER_CALLER_ARM_ENABLED) && UEXECUTER_CALLER_ARM_ENABLED
#include "cmsis_compiler.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

#if defined(__SOFT_FP__) || !defined(__ARM_FP)
#error "Caller for arm floating-point model is enabled but you are not using arm-fp-hard abi."
#endif

#if (__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define UNUSED __attribute__((unused))
#else
// may be helpful for some linter
#define UNUSED /*@unused@*/
#endif

#define NAKED __attribute__((naked))
#define AAPCS_VFP __attribute__((pcs("aapcs-vfp")))

NAKED AAPCS_VFP uint64_t uexecuter_arm_call(UNUSED uint32_t r0, UNUSED uint32_t r1, UNUSED uint32_t r2,
                                            UNUSED uint32_t r3, UNUSED double d0, UNUSED double d1, UNUSED double d2,
                                            UNUSED double d3, UNUSED double d4, UNUSED double d5, UNUSED double d6,
                                            UNUSED double d7, UNUSED void *function_ptr,
                                            UNUSED uint32_t n_params_in_stack, UNUSED uint32_t *params_in_stack,
                                            UNUSED void *d_out)
{
    __ASM("PUSH {R4,R5,R6,R7,LR}"); // use 20 bytes stack space
    __ASM("MOV R7, SP");            // backup stack point, in case that we passed too more or fewer parameters

    __ASM("LDR R4, [SP,#24]"); // R4 = n_params_in_stack
    __ASM("LDR R5, [SP,#28]"); // R5 = params_in_stack
    __ASM("ADD R5, R5, R4, LSL#2");

    __ASM("CMP R4, #0\n"
          "BEQ PUSH_PARAMS_END\n"
          "PUSH_PARAMS_IN_STACK:\n"
          "SUB R5, R5, #4\n"
          "SUB SP, SP, #4\n"
          "LDR R6, [R5]\n"
          "STR R6, [SP]\n"
          "SUBS R4, R4, #1\n"
          "BNE PUSH_PARAMS_IN_STACK\n"
          "PUSH_PARAMS_END:");

    __ASM("LDR R4, [R7,#20]"); // R4 = function_ptr
    __ASM("BLX R4");

    __ASM("LDR R5, [R7,#32]\n"
          "CMP R5, #0\n"
          "BEQ AFTER_WRITE_DOUBLE\n"
          "FSTD D0, [R5]\n"
          "AFTER_WRITE_DOUBLE:");

    __ASM("MOV SP, R7"); // restore stack point
    __ASM("POP {R4,R5,R6,R7,PC}");
}

__STATIC_FORCEINLINE uint64_t uexecuter_arm_call_no_float_params(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3,
                                                                 void *function_ptr, uint32_t n_params_in_stack,
                                                                 uint32_t *params_in_stack, double *d_out)
{
    // force to change the signature of `uexecuter_arm_call`, skipping the `LDR` for d0-d8 register
    // it's safe in ARM floating point model `fpv4-sp-d16`
    void *p_uexecuter_arm_call = uexecuter_arm_call;
    typedef uint64_t (*uexecuter_arm_call_no_double_params_t)(uint32_t, uint32_t, uint32_t, uint32_t, void *, uint32_t,
                                                              uint32_t *, double *);
    return ((uexecuter_arm_call_no_double_params_t)p_uexecuter_arm_call)(r0, r1, r2, r3, function_ptr,
                                                                         n_params_in_stack, params_in_stack, d_out);
}

typedef union {
    float f[2];
    double d;
} f32_or_f64_t;
#define N_INT_REGISTER_FOR_PARAMS 4
#define N_FLOAT_REGISTER_FOR_PARAMS 8
#define N_MAX_PARAMS_IN_STACK 16
uexecuter_caller_status_t uexecuter_call(void *function_ptr, const uexecuter_caller_param_t *params, size_t n_params,
                                         uexecuter_caller_result_t *result)
{
    result->u64 = -1;
    result->f64 = NAN;
    uint32_t r[N_INT_REGISTER_FOR_PARAMS];
    f32_or_f64_t d[N_FLOAT_REGISTER_FOR_PARAMS];
    uint32_t params_in_stack[N_MAX_PARAMS_IN_STACK];
    uint32_t n_params_in_stack = 0;
    uint32_t n_r_used = 0;
    uint8_t d_used[N_FLOAT_REGISTER_FOR_PARAMS] = {};
    for (int i = 0; i < n_params; ++i)
    {
        switch (params[i].type)
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_I32:
        case UEXECUTER_CALLER_TYPE_STR:
            if (n_r_used < sizeof(r))
            {
                r[n_r_used++] = params[i].u32;
            }
            else
            {
                if (n_params_in_stack >= N_MAX_PARAMS_IN_STACK)
                {
                    return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
                }
                params_in_stack[n_params_in_stack++] = params[i].u32;
            }
            break;
        case UEXECUTER_CALLER_TYPE_U64:
        case UEXECUTER_CALLER_TYPE_I64:
            if (n_r_used % 2)
            {
                n_r_used++;
            }
            if (n_r_used < N_INT_REGISTER_FOR_PARAMS)
            {
                r[n_r_used] = (uint32_t)(params[i].u64 & 0xFFFFFFFF);
                r[n_r_used + 1] = (uint32_t)(params[i].u64 >> 32);
                n_r_used += 2;
            }
            else
            {
                if (n_params_in_stack + 2 > N_MAX_PARAMS_IN_STACK)
                {
                    return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
                }
                memcpy(&params_in_stack[n_params_in_stack], &params[i].u64, sizeof(uint64_t));
                n_params_in_stack += 2;
            }
            break;
        case UEXECUTER_CALLER_TYPE_F32: {
            bool float_register_used = false;
            for (size_t i_d = 0; i_d < N_FLOAT_REGISTER_FOR_PARAMS; ++i_d)
            {
                if (d_used[i_d] != 2)
                {
                    d[i_d].f[d_used[i_d]++] = params[i].f32;
                    float_register_used = true;
                    break;
                }
            }
            if (!float_register_used)
            {
                if (n_params_in_stack >= N_MAX_PARAMS_IN_STACK)
                {
                    return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
                }
                params_in_stack[n_params_in_stack++] = params[i].u32;
            }
            break;
        }
        case UEXECUTER_CALLER_TYPE_F64: {
            bool float_register_used = false;
            for (size_t i_d = 0; i_d < N_FLOAT_REGISTER_FOR_PARAMS; ++i_d)
            {
                if (d_used[i_d] == 0)
                {
                    d[i_d].d = params[i].f64;
                    d_used[i_d] = 2;
                    float_register_used = true;
                    break;
                }
            }
            if (!float_register_used)
            {
                if (n_params_in_stack + 2 > N_MAX_PARAMS_IN_STACK)
                {
                    return UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS;
                }
                memcpy(&params_in_stack[n_params_in_stack], &params[i].f64, sizeof(double));
                n_params_in_stack += 2;
            }
            break;
        }
        default:
            return UEXECUTER_CALLER_STATUS_INVALID_PARAMETER;
        }
    }
    if (d_used[0] == 0)
    {
        // fast path
        result->u64 = uexecuter_arm_call_no_float_params(r[0], r[1], r[2], r[3], function_ptr, n_params_in_stack,
                                                         params_in_stack, &result->f64);
    }
    else
    {
        result->u64 = uexecuter_arm_call(r[0], r[1], r[2], r[3], d[0].d, d[1].d, d[2].d, d[3].d, d[4].d, d[5].d, d[6].d,
                                         d[7].d, function_ptr, n_params_in_stack, params_in_stack, &result->f64);
    }
    return UEXECUTER_CALLER_STATUS_OK;
}
#endif