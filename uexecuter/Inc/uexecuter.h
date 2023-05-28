#pragma once
#include "uexecuter_service.h"
#include "uexecuter_transport.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
#define UEXECUTER_BUFFER_SIZE ((size_t)256)
    typedef int uexecuter_status_t;
#define UEXECUTER_STATUS_END ((uexecuter_status_t)0)
#define UEXECUTER_STATUS_DATA ((uexecuter_status_t)1)
#define UEXECUTER_STATUS_OVERFLOW ((uexecuter_status_t)2)
    typedef struct
    {
        uexecuter_transport_t *transport;
        uexecuter_function_service_info_t service;
        uint8_t buf[UEXECUTER_BUFFER_SIZE];
        size_t buf_used;
        uexecuter_status_t status;
    } uexecuter_t;
    void uexecuter_init(uexecuter_t *inst, const uexecuter_function_prototype_t *function_prototypes,
                        size_t n_function);
    void uexecuter_handle_command(uexecuter_t *inst, uint8_t *command, size_t size);
    void uexecuter_bind(uexecuter_t *inst, uexecuter_transport_t *transport);
    void uexecuter_response(uexecuter_t *inst, const char *str);
    void uexecuter_response_char(uexecuter_t *inst, char c);
    void uexecuter_response_len(uexecuter_t *inst, const char *str, size_t len);
#ifdef __cplusplus
}
#endif