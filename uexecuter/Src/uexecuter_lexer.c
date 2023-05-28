#include "uexecuter_lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
static uint8_t dightsA[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static uint8_t dightsB[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static bool is_dight(uint8_t c, int base)
{
    for (int i = 0; i < base; ++i)
    {
        if (dightsA[i] == c || dightsB[i] == c)
            return true;
    }
    return false;
}

static bool is_identifier_begin(uint8_t c)
{
    if ('a' <= c && c <= 'z')
        return true;
    if ('A' <= c && c <= 'Z')
        return true;
    if (c == '_')
        return true;
    if (c == '$')
        return true;
    return false;
}

static bool is_identifier_part(uint8_t c)
{
    if ('a' <= c && c <= 'z')
        return true;
    if ('A' <= c && c <= 'Z')
        return true;
    if ('0' <= c && c <= '9')
        return true;
    if (c == '_')
        return true;
    if (c == '$')
        return true;
    return false;
}

// do not handle sign
static uint64_t string_to_u64(const uint8_t *str, const uint8_t *end, int base)
{
    int result = 0;

    while (str < end)
    {
        if ('0' <= *str && *str <= '9')
            result = result * base + (*str - '0');
        else if ('a' <= *str && *str <= 'z')
            result = result * base + (*str - 'a' + 10);
        else if ('A' <= *str && *str <= 'Z')
            result = result * base + (*str - 'A' + 10);
        else
            break;
        str++;
    }
    return result;
}

static double string_to_f64(const uint8_t *str, const uint8_t *end)
{
    char buffer[128];
    size_t size = end - str;
    memcpy(buffer, str, size);
    buffer[size] = '\0';
    return strtod(buffer, NULL);
}

static void uexecuter_lexer_parse_number(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst);
static void uexecuter_lexer_parse_string(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst);

static void integer_parsed_adaptive(uexecuter_lexer_token_t *dest, bool neg, const uint8_t *number_begin,
                                    const uint8_t *number_end, int base);

void uexecuter_lexer_init(uexecuter_lexer_t *inst, uint8_t *buf, size_t size)
{
    inst->cur = buf;
    inst->end = buf + size;
}

void uexecuter_lexer_next_token(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst)
{
    if (inst->cur == inst->end)
    {
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_EOF;
        return;
    }

    while (isspace(*inst->cur))
    {
        inst->cur++;
        if (inst->cur == inst->end)
        {
            dest->kind = UEXECUTER_LEXER_TOKEN_KIND_EOF;
            return;
        }
    }

    if (('0' <= *inst->cur && *inst->cur <= '9') || *inst->cur == '-' || *inst->cur == '+')
    {
        uexecuter_lexer_parse_number(dest, inst);
    }
    else if (*inst->cur == '(')
    {
        inst->cur++;
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_LEFT_PARENTHESIS;
    }
    else if (*inst->cur == ',')
    {
        inst->cur++;
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_COMMA;
    }
    else if (*inst->cur == ')')
    {
        inst->cur++;
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_RIGHT_PARENTHESIS;
    }
    else if (*inst->cur == '"')
    {
        uexecuter_lexer_parse_string(dest, inst);
    }
    else if (is_identifier_begin(*inst->cur))
    {
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_IDENTIFIER;
        dest->str.begin = (char *)inst->cur;
        do
        {
            inst->cur++;
        } while (inst->cur < inst->end && is_identifier_part(*inst->cur));
        dest->str.end = (char *)inst->cur;
    }
    else
    {
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_ERROR;
    }
}

static void uexecuter_lexer_parse_number(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst)
{
    bool neg = false;
    bool float_number = false;
    if (*inst->cur == '-')
    {
        inst->cur++;
        neg = true;
    }
    else if (*inst->cur == '+')
    {
        inst->cur++;
    }

    uint8_t *number_begin = inst->cur;
    int base = 10;
    // number
    inst->cur++;
    if (*number_begin == '0')
    {
        if ('0' <= *inst->cur && *inst->cur <= '7')
        {
            base = 8; // eg. `0777` == `511`
            number_begin = inst->cur;
        }
        else if (inst->cur + 1 < inst->end)
        {
            if (*inst->cur == 'x' || *inst->cur == 'X' && is_dight(inst->cur[1], 16))
            {
                base = 16;
                inst->cur++;
                number_begin = inst->cur;
            }
            else if (*inst->cur == 'b' || *inst->cur == 'B' && is_dight(inst->cur[1], 2))
            {
                base = 2;
                inst->cur++;
                number_begin = inst->cur;
            }
            else if (*inst->cur == 'o' || *inst->cur == 'O' && is_dight(inst->cur[1], 8))
            {
                base = 8;
                inst->cur++;
                number_begin = inst->cur;
            }
        }
    }
    while (inst->cur < inst->end && is_dight(*inst->cur, base))
    {
        inst->cur++;
    }
    if (inst->cur >= inst->end)
    {
        // end here, then it must be an integer
        return integer_parsed_adaptive(dest, neg, number_begin, inst->cur, base);
    }
    else
    {
        if (*inst->cur == '.')
        {
            float_number = true;
            inst->cur++;
            while (inst->cur < inst->end && is_dight(*inst->cur, base))
            {
                inst->cur++;
            }
        }
        else if (*inst->cur == 'e' || *inst->cur == 'E')
        {
            float_number = true;
            inst->cur++;
            if (*inst->cur == '-' || *inst->cur == '+')
            {
                inst->cur++;
            }
            while (inst->cur < inst->end && is_dight(*inst->cur, base))
            {
                inst->cur++;
            }
        }
        if (float_number)
        {
            dest->kind = UEXECUTER_LEXER_TOKEN_KIND_F64;
            dest->f64 = string_to_f64(number_begin, inst->cur);
            if (neg)
            {
                dest->f64 = -dest->f64;
            }
            if (inst->cur < inst->end)
            {
                if (*inst->cur == 'f' || *inst->cur == 'F')
                {
                    inst->cur++;
                    dest->kind = UEXECUTER_LEXER_TOKEN_KIND_F32;
                    dest->f32 = (float)dest->f64;
                }
            }
            return;
        }
        else
        {
            // for non-float number, we should have meet `inst->cur < inst->end`
            if (*inst->cur == 'l' || *inst->cur == 'L')
            {
                dest->kind = UEXECUTER_LEXER_TOKEN_KIND_U64;
                dest->u64 = string_to_u64(number_begin, inst->cur, base);
                inst->cur++;
                return;
            }
            return integer_parsed_adaptive(dest, neg, number_begin, inst->cur, base);
        }
    }
}

// 启发式地判断是否为 64bit or 32bit 数
static void integer_parsed_adaptive(uexecuter_lexer_token_t *dest, bool neg, const uint8_t *number_begin,
                                    const uint8_t *number_end, int base)
{
    dest->kind = UEXECUTER_LEXER_TOKEN_KIND_U64;
    dest->u64 = string_to_u64(number_begin, number_end, base);
    if (neg)
    {
        dest->u64 = (~dest->u64) + 1;
    }
    if ((dest->u64 & UINT64_C(0xFFFFFFFF00000000)) == 0 ||
        (dest->u64 & UINT64_C(0xFFFFFFFF00000000)) == UINT64_C(0xFFFFFFFF00000000))
    {
        dest->kind = UEXECUTER_LEXER_TOKEN_KIND_U32;
    }
}

static void uexecuter_lexer_parse_string(uexecuter_lexer_token_t *dest, uexecuter_lexer_t *inst)
{
    inst->cur++; // skip the first quota
    dest->kind = UEXECUTER_LEXER_TOKEN_KIND_STR;
    dest->str.begin = (char *)inst->cur;
    char *next_char = (char *)inst->cur;
    for (;;)
    {
        if (inst->cur >= inst->end)
        {
            dest->kind = UEXECUTER_LEXER_TOKEN_KIND_ERROR;
            return;
        }
        if (*inst->cur == '"')
        {
            inst->cur++;
            break;
        }
        else if (*inst->cur == '\\')
        {
            // handle escaped string
            if (inst->cur + 1 >= inst->end)
            {
                dest->kind = UEXECUTER_LEXER_TOKEN_KIND_ERROR;
                return;
            }
            inst->cur++;
            switch (*inst->cur)
            {
            case 'n':
                *next_char = '\n';
                inst->cur++;
                next_char++;
                break;
            case 'r':
                *next_char = '\r';
                inst->cur++;
                next_char++;
                break;
            case 't':
                *next_char = '\t';
                inst->cur++;
                next_char++;
                break;
            case 'f':
                *next_char = '\f';
                inst->cur++;
                next_char++;
                break;
            default:
                dest->kind = UEXECUTER_LEXER_TOKEN_KIND_ERROR;
                return;
            }
            continue;
        }
        if (next_char != (char *)inst->cur)
        {
            *next_char = (char)*inst->cur;
        }
        inst->cur++;
        next_char++;
    }
    dest->str.end = next_char;
}