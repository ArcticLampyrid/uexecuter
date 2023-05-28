#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        uint8_t *cur;
        uint8_t *end; // not included
    } uexecuter_lexer_t;
    typedef int uexecuter_lexer_token_kind_t;
#define UEXECUTER_LEXER_TOKEN_KIND_ERROR ((uexecuter_lexer_token_kind_t)-1)
#define UEXECUTER_LEXER_TOKEN_KIND_U32 ((uexecuter_lexer_token_kind_t)0)
#define UEXECUTER_LEXER_TOKEN_KIND_F32 ((uexecuter_lexer_token_kind_t)1)
#define UEXECUTER_LEXER_TOKEN_KIND_U64 ((uexecuter_lexer_token_kind_t)2)
#define UEXECUTER_LEXER_TOKEN_KIND_F64 ((uexecuter_lexer_token_kind_t)3)
#define UEXECUTER_LEXER_TOKEN_KIND_STR ((uexecuter_lexer_token_kind_t)4)
#define UEXECUTER_LEXER_TOKEN_KIND_IDENTIFIER ((uexecuter_lexer_token_kind_t)5)
#define UEXECUTER_LEXER_TOKEN_KIND_COMMA ((uexecuter_lexer_token_kind_t)6)
#define UEXECUTER_LEXER_TOKEN_KIND_LEFT_PARENTHESIS ((uexecuter_lexer_token_kind_t)7)
#define UEXECUTER_LEXER_TOKEN_KIND_RIGHT_PARENTHESIS ((uexecuter_lexer_token_kind_t)8)
#define UEXECUTER_LEXER_TOKEN_KIND_EOF ((uexecuter_lexer_token_kind_t)9)

    typedef struct
    {
        uexecuter_lexer_token_kind_t kind;
        union {
            uint32_t u32;
            float f32;
            uint64_t u64;
            double f64;
            struct
            {
                char *begin;
                char *end;
            } str;
        };
    } uexecuter_lexer_token_t;

    void uexecuter_lexer_init(uexecuter_lexer_t *inst, uint8_t *buf, size_t size);
    void uexecuter_lexer_next_token(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst);
#ifdef __cplusplus
}
#endif