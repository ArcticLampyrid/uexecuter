#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
    struct uexecuter_transport_s;
    typedef struct uexecuter_transport_s uexecuter_transport_t;

    typedef void (*uexecuter_transport_on_received_cb_t)(uint8_t c, void *userdata);
    typedef void (*uexecuter_transport_transmit_t)(uexecuter_transport_t *inst, uint8_t c);
    struct uexecuter_transport_s
    {
        uexecuter_transport_on_received_cb_t on_received;
        uexecuter_transport_transmit_t transmit;
        void *userdata;
    };
    void uexecuter_transport_on_received_cb_empty(uint8_t c, void *userdata);
#ifdef __cplusplus
}
#endif