#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

    void app_pre_init();

    void app_init();

    void app_sys_init();

    __attribute__((noreturn)) void app_main();

    void usart1_irq();
#ifdef __cplusplus
}
#endif