#pragma once
#if defined(STM32F4) || defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) ||                        \
    defined(STM32F417xx) || defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) ||                    \
    defined(STM32F439xx) || defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F410Tx) ||                    \
    defined(STM32F410Cx) || defined(STM32F410Rx) || defined(STM32F411xE) || defined(STM32F446xx) ||                    \
    defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F412Cx) || defined(STM32F412Rx) ||                    \
    defined(STM32F412Vx) || defined(STM32F412Zx) || defined(STM32F413xx) || defined(STM32F423xx)
#include "stm32f4xx.h"
#else
#error "Unsupported STM32 platform"
#endif
#include "uexecuter_transport.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        uexecuter_transport_t base;
        USART_TypeDef *usart;
    } uexecuter_transport_stm32_t;
    void uexecuter_transport_stm32_init(uexecuter_transport_stm32_t *inst, USART_TypeDef *usart);
    void uexecuter_transport_stm32_begin(uexecuter_transport_stm32_t *inst);
    void uexecuter_transport_stm32_handle_isr(uexecuter_transport_stm32_t *inst);
#ifdef __cplusplus
}
#endif