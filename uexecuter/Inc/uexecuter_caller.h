#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef uint8_t uexecuter_caller_type_t;

#define UEXECUTER_CALLER_TYPE_VOID ((uexecuter_caller_type_t)0xFF)

#define UEXECUTER_CALLER_TYPE_U32 ((uexecuter_caller_type_t)0)
#define UEXECUTER_CALLER_TYPE_F32 ((uexecuter_caller_type_t)1)
#define UEXECUTER_CALLER_TYPE_U64 ((uexecuter_caller_type_t)2)
#define UEXECUTER_CALLER_TYPE_F64 ((uexecuter_caller_type_t)3)

// 区分一下，主要是为了显示 Result 的时候的效果
#define UEXECUTER_CALLER_TYPE_I32 ((uexecuter_caller_type_t)4)
#define UEXECUTER_CALLER_TYPE_I64 ((uexecuter_caller_type_t)5)
#define UEXECUTER_CALLER_TYPE_STR ((uexecuter_caller_type_t)6)

    typedef int8_t uexecuter_caller_status_t;
#define UEXECUTER_CALLER_STATUS_OK ((uexecuter_caller_status_t)0)
#define UEXECUTER_CALLER_STATUS_FP_NOT_SUPPORTED ((uexecuter_caller_status_t)-1)
#define UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS ((uexecuter_caller_status_t)-2)
#define UEXECUTER_CALLER_STATUS_INVALID_PARAMETER ((uexecuter_caller_status_t)-3)

    typedef struct
    {
        uexecuter_caller_type_t type;
        union {
            uint32_t u32;
            int32_t i32;
            float f32;
            uint64_t u64;
            int64_t i64;
            double f64;
            void *ptr;
        };
    } uexecuter_caller_param_t;

    typedef struct
    {
        // value from R0/R1
        union {
            uint32_t u32;
            int32_t i32;
            uint64_t u64;
            int64_t i64;
            void *ptr;
        };
        // value from float-pointing register
        union {
            float f32;
            double f64;
        };
    } uexecuter_caller_result_t;

    uexecuter_caller_status_t uexecuter_call(void *function_ptr, const uexecuter_caller_param_t *params,
                                             size_t n_params, uexecuter_caller_result_t *result);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ only part

template <typename T> struct uexecuter_caller_type_id_of
{
};

template <> struct uexecuter_caller_type_id_of<void>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_VOID;
};

template <> struct uexecuter_caller_type_id_of<uint8_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_U32;
};

template <> struct uexecuter_caller_type_id_of<int8_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_I32;
};

template <> struct uexecuter_caller_type_id_of<uint16_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_U32;
};

template <> struct uexecuter_caller_type_id_of<int16_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_I32;
};

template <> struct uexecuter_caller_type_id_of<uint32_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_U32;
};

template <> struct uexecuter_caller_type_id_of<int32_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_I32;
};

template <> struct uexecuter_caller_type_id_of<uint64_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_U64;
};

template <> struct uexecuter_caller_type_id_of<int64_t>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_I64;
};

template <> struct uexecuter_caller_type_id_of<float>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_F32;
};

template <> struct uexecuter_caller_type_id_of<double>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_F64;
};

template <> struct uexecuter_caller_type_id_of<const char *>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_STR;
};

template <> struct uexecuter_caller_type_id_of<char *>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_STR;
};

template <> struct uexecuter_caller_type_id_of<const uint8_t *>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_STR;
};

template <> struct uexecuter_caller_type_id_of<uint8_t *>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_STR;
};

template <> struct uexecuter_caller_type_id_of<void *>
{
    static constexpr const uexecuter_caller_type_t value = UEXECUTER_CALLER_TYPE_U32;
};
#endif

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#define UEXECUTER_CALLER_ARM_ENABLED 1
#else
#define UEXECUTER_CALLER_GENERIC_ENABLED 1
#endif