#include "uexecuter_transport_stm32.h"
static void uexecuter_transport_stm32_transmit(uexecuter_transport_stm32_t *inst, uint8_t c)
{
    while ((inst->usart->SR & USART_SR_TXE) == 0)
        ;
    inst->usart->DR = c;
}

void uexecuter_transport_stm32_init(uexecuter_transport_stm32_t *inst, USART_TypeDef *usart)
{
    inst->base.on_received = uexecuter_transport_on_received_cb_empty;
    inst->base.transmit = (uexecuter_transport_transmit_t)&uexecuter_transport_stm32_transmit;
    inst->usart = usart;
}

void uexecuter_transport_stm32_begin(uexecuter_transport_stm32_t *inst)
{
    inst->usart->CR1 |= USART_CR1_RXNEIE;
    inst->usart->CR3 |= USART_CR3_EIE;
}
void uexecuter_transport_stm32_handle_isr(uexecuter_transport_stm32_t *inst)
{
    uint32_t isrflags = READ_REG(inst->usart->SR);
    uint32_t cr1its = READ_REG(inst->usart->CR1);
    if ((isrflags & USART_SR_RXNE) && (cr1its & USART_CR1_RXNEIE))
    {
        READ_REG(inst->usart->SR);         // Read status register
        unsigned char c = inst->usart->DR; // Read data register
        inst->base.on_received(c, inst->base.userdata);
    }
    else if (isrflags & (USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE))
    {
        // Flags will be cleared after reading SR and then DR
        // Here we just ignore them
        // See also: Page 516, RM0368 Rev 5
        READ_REG(inst->usart->SR); // Read status register
        READ_REG(inst->usart->DR); // Read data register
    }
}
