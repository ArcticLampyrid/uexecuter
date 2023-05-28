#include "uexecuter.h"
#include "uexecuter_lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define UEXECUTER_ALLOW_UNPROTOTYPED_CALL 1

void uexecuter_init(uexecuter_t *inst, const uexecuter_function_prototype_t *function_prototypes, size_t n_function)
{
    inst->transport = NULL;
    inst->service.function_prototypes = function_prototypes;
    inst->service.n_function = n_function;
    inst->buf_used = 0;
    inst->status = UEXECUTER_STATUS_END;
}

static bool is_a_begin(uint8_t c)
{
    if ('a' <= c && c <= 'z')
        return true;
    if ('A' <= c && c <= 'Z')
        return true;
    if ('0' <= c && c <= '9') // For calling via address, cmd is beginning with a number
        return true;
    if (c == '_')
        return true;
    if (c == '$')
        return true;
    return false;
}

static bool is_a_end(uint8_t c)
{
    return c == '\r' || c == '\n';
}

void uexecuter_response(uexecuter_t *inst, const char *str)
{
    uexecuter_transport_t *transport = inst->transport;
    for (char c = *str; c != '\0'; c = *(++str))
    {
        transport->transmit(transport, (uint8_t)c);
    }
}

void uexecuter_response_char(uexecuter_t *inst, char c)
{
    uexecuter_transport_t *transport = inst->transport;
    transport->transmit(transport, (uint8_t)c);
}

void uexecuter_response_len(uexecuter_t *inst, const char *str, size_t len)
{
    uexecuter_transport_t *transport = inst->transport;
    const char *end = str + len;
    while (str != end)
    {
        transport->transmit(transport, (uint8_t)*str);
        str++;
    }
}

#define N_PARAMS_LIMIT 20

static bool uexecuter_get_param_from_u32_token(uexecuter_caller_param_t *dest, uexecuter_lexer_token_t *token,
                                               const uexecuter_function_prototype_t *prototype, size_t i_params)
{
    if (prototype != NULL)
    {
        switch (prototype->param_type[i_params])
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_I32:
        case UEXECUTER_CALLER_TYPE_STR:
            dest->type = prototype->param_type[i_params];
            dest->u32 = token->u32;
            break;
        case UEXECUTER_CALLER_TYPE_U64:
        case UEXECUTER_CALLER_TYPE_I64:
            dest->type = prototype->param_type[i_params];
            dest->u64 = token->u32;
            break;
        case UEXECUTER_CALLER_TYPE_F32:
            dest->type = UEXECUTER_CALLER_TYPE_F32;
            dest->f32 = (float)token->u32;
            break;
        case UEXECUTER_CALLER_TYPE_F64:
            dest->type = UEXECUTER_CALLER_TYPE_F64;
            dest->f64 = (double)token->u32;
            break;
        default:
            return false;
        }
    }
    else
    {
        dest->type = UEXECUTER_CALLER_TYPE_U32;
        dest->u32 = token->u32;
    }
    return true;
}

static bool uexecuter_get_param_from_u64_token(uexecuter_caller_param_t *dest, uexecuter_lexer_token_t *token,
                                               const uexecuter_function_prototype_t *prototype, size_t i_params)
{
    if (prototype != NULL)
    {
        switch (prototype->param_type[i_params])
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_I32:
        case UEXECUTER_CALLER_TYPE_STR:
            dest->type = prototype->param_type[i_params];
            dest->u32 = (uint32_t)token->u64;
            break;
        case UEXECUTER_CALLER_TYPE_U64:
        case UEXECUTER_CALLER_TYPE_I64:
            dest->type = prototype->param_type[i_params];
            dest->u64 = token->u64;
            break;
        case UEXECUTER_CALLER_TYPE_F32:
            dest->type = UEXECUTER_CALLER_TYPE_F32;
            dest->f32 = (float)token->u64;
            break;
        case UEXECUTER_CALLER_TYPE_F64:
            dest->type = UEXECUTER_CALLER_TYPE_F64;
            dest->f64 = (double)token->u64;
            break;
        default:
            return false;
        }
    }
    else
    {
        dest->type = UEXECUTER_CALLER_TYPE_U64;
        dest->u64 = token->u64;
    }
    return true;
}

static bool uexecuter_get_param_from_f32_token(uexecuter_caller_param_t *dest, uexecuter_lexer_token_t *token,
                                               const uexecuter_function_prototype_t *prototype, size_t i_params)
{
    if (prototype != NULL)
    {
        switch (prototype->param_type[i_params])
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_STR:
            dest->type = UEXECUTER_CALLER_TYPE_U32;
            dest->u32 = (uint64_t)token->f32;
            break;
        case UEXECUTER_CALLER_TYPE_I32:
            dest->type = UEXECUTER_CALLER_TYPE_I32;
            dest->i32 = (int32_t)token->f32;
            break;
        case UEXECUTER_CALLER_TYPE_U64:
            dest->type = UEXECUTER_CALLER_TYPE_U64;
            dest->u64 = (uint64_t)token->f32;
            break;
        case UEXECUTER_CALLER_TYPE_I64:
            dest->type = UEXECUTER_CALLER_TYPE_I64;
            dest->i64 = (int64_t)token->f32;
            break;
        case UEXECUTER_CALLER_TYPE_F32:
            dest->type = UEXECUTER_CALLER_TYPE_F32;
            dest->f32 = token->f32;
            break;
        case UEXECUTER_CALLER_TYPE_F64:
            dest->type = UEXECUTER_CALLER_TYPE_F64;
            dest->f64 = (double)token->f32;
            break;
        default:
            return false;
        }
    }
    else
    {
        dest->type = UEXECUTER_CALLER_TYPE_F32;
        dest->f32 = token->f32;
    }
    return true;
}

static bool uexecuter_get_param_from_f64_token(uexecuter_caller_param_t *dest, uexecuter_lexer_token_t *token,
                                               const uexecuter_function_prototype_t *prototype, size_t i_params)
{
    if (prototype != NULL)
    {
        switch (prototype->param_type[i_params])
        {
        case UEXECUTER_CALLER_TYPE_U32:
        case UEXECUTER_CALLER_TYPE_STR:
            dest->type = UEXECUTER_CALLER_TYPE_U32;
            dest->u32 = (uint64_t)token->f64;
            break;
        case UEXECUTER_CALLER_TYPE_I32:
            dest->type = UEXECUTER_CALLER_TYPE_I32;
            dest->i32 = (int32_t)token->f64;
            break;
        case UEXECUTER_CALLER_TYPE_U64:
            dest->type = UEXECUTER_CALLER_TYPE_U64;
            dest->u64 = (uint64_t)token->f64;
            break;
        case UEXECUTER_CALLER_TYPE_I64:
            dest->type = UEXECUTER_CALLER_TYPE_I64;
            dest->i64 = (int64_t)token->f64;
            break;
        case UEXECUTER_CALLER_TYPE_F32:
            dest->type = UEXECUTER_CALLER_TYPE_F32;
            dest->f32 = (float)token->f64;
            break;
        case UEXECUTER_CALLER_TYPE_F64:
            dest->type = UEXECUTER_CALLER_TYPE_F64;
            dest->f64 = token->f64;
            break;
        default:
            return false;
        }
    }
    else
    {
        dest->type = UEXECUTER_CALLER_TYPE_F64;
        dest->f64 = token->f64;
    }
    return true;
}

static void uexecuter_print_type(uexecuter_t *inst, uexecuter_caller_type_t t)
{
    switch (t)
    {
    case UEXECUTER_CALLER_TYPE_VOID:
        uexecuter_response(inst, "void");
        break;
    case UEXECUTER_CALLER_TYPE_U32:
        uexecuter_response(inst, "u32");
        break;
    case UEXECUTER_CALLER_TYPE_F32:
        uexecuter_response(inst, "f32");
        break;
    case UEXECUTER_CALLER_TYPE_U64:
        uexecuter_response(inst, "u64");
        break;
    case UEXECUTER_CALLER_TYPE_F64:
        uexecuter_response(inst, "f64");
        break;
    case UEXECUTER_CALLER_TYPE_I32:
        uexecuter_response(inst, "i32");
        break;
    case UEXECUTER_CALLER_TYPE_I64:
        uexecuter_response(inst, "i64");
        break;
    case UEXECUTER_CALLER_TYPE_STR:
        uexecuter_response(inst, "str");
        break;
    default:
        uexecuter_response(inst, "<unk>");
        break;
    }
}

void uexecuter_handle_list_instruction(uexecuter_t *inst)
{
    char repr[12];
    for (size_t i = 0; i < inst->service.n_function; i++)
    {
        const uexecuter_function_prototype_t *prototype = &inst->service.function_prototypes[i];
        uexecuter_response(inst, "UEXECUTER: [#");
        sprintf(repr, "%zu", i + 1);
        uexecuter_response(inst, repr);
        uexecuter_response(inst, "] ");
        uexecuter_print_type(inst, prototype->result_type);
        uexecuter_response_char(inst, ' ');
        uexecuter_response_len(inst, prototype->name, prototype->name_len);
        uexecuter_response_char(inst, '(');
        if (prototype->n_params >= 1)
        {
            uexecuter_print_type(inst, prototype->param_type[0]);
            for (size_t i_params = 1; i_params < prototype->n_params; i_params++)
            {
                uexecuter_response_char(inst, ',');
                uexecuter_response_char(inst, ' ');
                uexecuter_print_type(inst, prototype->param_type[i_params]);
            }
        }
        uexecuter_response_char(inst, ')');
        uexecuter_response_char(inst, '\n');
    }
}

static void uexecuter_handle_id_instruction(uexecuter_t *inst)
{
    char repr[12];
    for (size_t i = 0; i < inst->service.n_function; i++)
    {
        const uexecuter_function_prototype_t *prototype = &inst->service.function_prototypes[i];
        uexecuter_response(inst, "UEXECUTER: [#");
        sprintf(repr, "%zu", i + 1);
        uexecuter_response(inst, repr);
        uexecuter_response(inst, "] ");
        uexecuter_response_len(inst, prototype->name, prototype->name_len);
        uexecuter_response(inst, " => ");
        sprintf(repr, "%p", prototype->ptr);
        uexecuter_response(inst, repr);
        uexecuter_response_char(inst, '\n');
    }
}

void uexecuter_handle_command(uexecuter_t *inst, uint8_t *command, size_t size)
{
    uexecuter_lexer_t lexer;
    uexecuter_lexer_init(&lexer, command, size);
    uexecuter_lexer_token_t token;
    uexecuter_lexer_next_token(&token, &lexer);

    uexecuter_caller_param_t params[N_PARAMS_LIMIT];
    size_t n_params = 0;

    void *function_ptr;
    const uexecuter_function_prototype_t *prototype;
    switch (token.kind)
    {
    case UEXECUTER_LEXER_TOKEN_KIND_IDENTIFIER:
        if (token.str.end - token.str.begin == 2 && memcmp(token.str.begin, "id", 2) == 0)
        {
            uexecuter_handle_id_instruction(inst);
            return;
        }
        else if (token.str.end - token.str.begin == 4 && memcmp(token.str.begin, "list", 4) == 0)
        {
            uexecuter_handle_list_instruction(inst);
            return;
        }
        prototype = uexecuter_service_find_by_name(&inst->service, token.str.begin, token.str.end);
        if (prototype == NULL)
        {
            uexecuter_response(inst, "UEXECUTER: failed to find the function by name\n");
            return;
        }
        function_ptr = prototype->ptr;
        break;
    case UEXECUTER_LEXER_TOKEN_KIND_U32:
    case UEXECUTER_LEXER_TOKEN_KIND_U64:
        function_ptr = (void *)(token.u32);
        prototype = uexecuter_service_find_by_address(&inst->service, function_ptr);
        if (prototype == NULL)
        {
#if defined(UEXECUTER_ALLOW_UNPROTOTYPED_CALL) && UEXECUTER_ALLOW_UNPROTOTYPED_CALL
            uexecuter_response(inst, "UEXECUTER: failed to find the function by addr, do unprototyped call (unsafe)\n");
#else
            uexecuter_response(inst, "UEXECUTER: failed to find the function by addr\n");
            return;
#endif
        }
        break;
    case UEXECUTER_LEXER_TOKEN_KIND_EOF:
        // do not report the error for a string filled with space chars
        return;
    default:
        uexecuter_response(inst, "UEXECUTER: missing identifier or address\n");
        return;
    }
    uexecuter_lexer_next_token(&token, &lexer);
    if (token.kind != UEXECUTER_LEXER_TOKEN_KIND_LEFT_PARENTHESIS)
    {
        uexecuter_response(inst, "UEXECUTER: missing left parenthesis\n");
        return;
    }

    uexecuter_lexer_next_token(&token, &lexer);
    while (token.kind != UEXECUTER_LEXER_TOKEN_KIND_RIGHT_PARENTHESIS)
    {
        switch (token.kind)
        {
        case UEXECUTER_LEXER_TOKEN_KIND_U32: {
            size_t i_params = n_params++;
            if (!uexecuter_get_param_from_u32_token(&params[i_params], &token, prototype, i_params))
            {
                uexecuter_response(inst, "UEXECUTER: expect parameter but found something else\n");
                return;
            }
            break;
        }
        case UEXECUTER_LEXER_TOKEN_KIND_F32: {
            size_t i_params = n_params++;
            if (!uexecuter_get_param_from_f32_token(&params[i_params], &token, prototype, i_params))
            {
                uexecuter_response(inst, "UEXECUTER: expect parameter but found something else\n");
                return;
            }
            break;
        }
        case UEXECUTER_LEXER_TOKEN_KIND_U64: {
            size_t i_params = n_params++;
            if (!uexecuter_get_param_from_u64_token(&params[i_params], &token, prototype, i_params))
            {
                uexecuter_response(inst, "UEXECUTER: expect parameter but found something else\n");
                return;
            }
            break;
        }
        case UEXECUTER_LEXER_TOKEN_KIND_F64: {
            size_t i_params = n_params++;
            if (!uexecuter_get_param_from_f64_token(&params[i_params], &token, prototype, i_params))
            {
                uexecuter_response(inst, "UEXECUTER: expect parameter but found something else\n");
                return;
            }
            break;
        }
        case UEXECUTER_LEXER_TOKEN_KIND_STR: {
            // change in place
            // it's safe for we always have an end marking (quota) after a string
            // no unexpected memory block will be accessed
            *token.str.end = '\0';

            size_t i_params = n_params++;
            uexecuter_caller_param_t *param = &params[i_params];
            param->type = UEXECUTER_CALLER_TYPE_U32;
            param->ptr = token.str.begin;
            break;
        }
        default:
            uexecuter_response(inst, "UEXECUTER: expect parameter but found something else\n");
            return;
        }

        uexecuter_lexer_next_token(&token, &lexer);
        if (token.kind != UEXECUTER_LEXER_TOKEN_KIND_COMMA &&
            token.kind != UEXECUTER_LEXER_TOKEN_KIND_RIGHT_PARENTHESIS)
        {
            uexecuter_response(inst, "UEXECUTER: missing comma or right parenthesis\n");
            return;
        }
        else if (token.kind == UEXECUTER_LEXER_TOKEN_KIND_COMMA)
        {
            uexecuter_lexer_next_token(&token, &lexer);
        }
    }

    if (prototype != NULL && n_params != prototype->n_params)
    {
        uexecuter_response(inst, "UEXECUTER: the number of parameters mismatched\n");
        return;
    }

    uexecuter_caller_result_t function_result;
    uexecuter_caller_status_t caller_status = uexecuter_call(function_ptr, params, n_params, &function_result);
    switch (caller_status)
    {
    case UEXECUTER_CALLER_STATUS_OK:
        break;
    case UEXECUTER_CALLER_STATUS_FP_NOT_SUPPORTED:
        uexecuter_response(inst, "UEXECUTER: failed to execute for floating-point not supported");
        return;
    case UEXECUTER_CALLER_STATUS_TOO_MANY_PARAMETERS:
        uexecuter_response(inst, "UEXECUTER: failed to execute for too many parameters");
        return;
    case UEXECUTER_CALLER_STATUS_INVALID_PARAMETER:
        uexecuter_response(inst, "UEXECUTER: failed to execute for invalid parameter");
        return;
    default:
        uexecuter_response(inst, "UEXECUTER: failed to execute for unknown reason");
        return;
    }

    char result_stat_str[160];
    if (prototype != NULL)
    {
        switch (prototype->result_type)
        {
        case UEXECUTER_CALLER_TYPE_VOID:
            uexecuter_response(inst, "UEXECUTER: executed, result = void\n");
            return;
        case UEXECUTER_CALLER_TYPE_U32:
            sprintf(result_stat_str, "UEXECUTER: executed, result = u32(%lu)\n", function_result.u32);
            uexecuter_response(inst, result_stat_str);
            return;
        case UEXECUTER_CALLER_TYPE_I32:
            sprintf(result_stat_str, "UEXECUTER: executed, result = i32(%ld)\n", function_result.i32);
            uexecuter_response(inst, result_stat_str);
            return;
        case UEXECUTER_CALLER_TYPE_STR:
            uexecuter_response(inst, "UEXECUTER: executed, result = str(");
            uexecuter_response(inst, (const char *)function_result.ptr);
            uexecuter_response(inst, ")");
            return;
        case UEXECUTER_CALLER_TYPE_U64:
            sprintf(result_stat_str, "UEXECUTER: executed, u64(%llu)\n", function_result.u64);
            uexecuter_response(inst, result_stat_str);
            return;
        case UEXECUTER_CALLER_TYPE_I64:
            sprintf(result_stat_str, "UEXECUTER: executed, i64(%lld)\n", function_result.i64);
            uexecuter_response(inst, result_stat_str);
            return;
        case UEXECUTER_CALLER_TYPE_F32:
            sprintf(result_stat_str, "UEXECUTER: executed, result = f32(%.9g)\n", function_result.f32);
            uexecuter_response(inst, result_stat_str);
            return;
        case UEXECUTER_CALLER_TYPE_F64:
            sprintf(result_stat_str, "UEXECUTER: executed, result = f64(%.17g)\n", function_result.f64);
            uexecuter_response(inst, result_stat_str);
            return;
        }
    }
    sprintf(result_stat_str,
            "UEXECUTER: executed, result = u32(%lu), i32(%ld), u64(%llu), i64(%lld), f32(%.9g), f64(%.17g)\n",
            function_result.u32, function_result.i32, function_result.u64, function_result.i64, function_result.f32,
            function_result.f64);
    uexecuter_response(inst, result_stat_str);
}

static void uexecuter_on_received(uint8_t x, uexecuter_t *inst)
{
    switch (inst->status)
    {
    case UEXECUTER_STATUS_END:
        if (is_a_begin(x))
        {
            inst->status = UEXECUTER_STATUS_DATA;
            inst->buf_used = 1;
            inst->buf[0] = x;
        }
        break;
    case UEXECUTER_STATUS_DATA:
        if (is_a_end(x))
        {
            inst->status = UEXECUTER_STATUS_END;
            // the end char is not needed to be included in the buffer

            // note that we do not use null-terminated here,
            // since we can get the size field easily.

            // the buffer will be in an uncertain state after handling the command
            // we may do some in-place parsing, eg. `f("abc\r\n")` --> `f("abc<CR><LF><NUL>n")`
            uexecuter_handle_command(inst, inst->buf, inst->buf_used);
        }
        else
        {
            if (inst->buf_used == UEXECUTER_BUFFER_SIZE)
            {
                inst->status = UEXECUTER_STATUS_OVERFLOW;
            }
            else
            {
                inst->buf[inst->buf_used++] = x;
            }
        }
        break;
    case UEXECUTER_STATUS_OVERFLOW:
        if (is_a_end(x))
        {
            inst->status = UEXECUTER_STATUS_END;
            // skip this command since the buffer is overflow
        }
        break;
    }
}

void uexecuter_bind(uexecuter_t *inst, uexecuter_transport_t *transport)
{
    transport->userdata = inst;
    transport->on_received = (uexecuter_transport_on_received_cb_t)uexecuter_on_received;
    inst->transport = transport;
}