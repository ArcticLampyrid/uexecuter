#include "uexecuter_tick.h"
#ifdef USE_HAL_DRIVER
#if defined(STM32F4) || defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) ||                        \
    defined(STM32F417xx) || defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) ||                    \
    defined(STM32F439xx) || defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F410Tx) ||                    \
    defined(STM32F410Cx) || defined(STM32F410Rx) || defined(STM32F411xE) || defined(STM32F446xx) ||                    \
    defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F412Cx) || defined(STM32F412Rx) ||                    \
    defined(STM32F412Vx) || defined(STM32F412Zx) || defined(STM32F413xx) || defined(STM32F423xx)
#include "stm32f4xx_hal.h"
#define UEXECUTER_TICK_USE_HAL 1
#endif
#endif
#if defined(UEXECUTER_TICK_USE_HAL) && UEXECUTER_TICK_USE_HAL
uint32_t uexecuter_tick_us()
{
    uint32_t m0 = HAL_GetTick();
    __IO uint32_t u0 = SysTick->VAL;
    uint32_t m1 = HAL_GetTick();
    __IO uint32_t u1 = SysTick->VAL;
    const uint32_t tms = SysTick->LOAD + 1;
    if (m1 != m0)
    {
        return (m1 * 1000 + ((tms - u1) * 1000) / tms);
    }
    else
    {
        return (m0 * 1000 + ((tms - u0) * 1000) / tms);
    }
}
#elif (!define(UEXECUTER_TICK_NO_FALLBACK)) || (!UEXECUTER_TICK_NO_FALLBACK)
// Do not define the fallback always, even if it's marked `weak`.
// GCC doesn't has a satisfactory implementation of `weak`
// See also: https://stackoverflow.com/a/37191811/5549179

#if defined(__GNUC__) || defined(__ARMCC_VERSION) || defined(__ADSPBLACKFIN__)
#define WEAK_FIRST __attribute__((weak))
#elif defined(__CC_ARM) || defined(__IAR_SYSTEMS_ICC__) || defined(__weak)
#define WEAK_FIRST __weak
#else
#define WEAK_FIRST
#endif
WEAK_FIRST uint32_t uexecuter_tick_us()
{
    return 0;
}
#endif