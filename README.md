# UEXECUTER
Languages: [Chinese](README_zh.md) | [English](README.md)

UEXECUTER is a cute function caller, which takes human-readable strings as instructions, and calls functions on the microcontroller via serial communication. Currently, it supports STM32F4 series microcontrollers and ARM hardware floating point model (`-mfloat-abi=hard`).

## Features
- Human-readable instructions
- Support for ARM hardware floating point model
- Easy to use

## Usage
1. Define the service and register your function: 
   ```cpp
   double add_f32_f64_i32(float x, double y, int32_t z)
   {
       return x + y + z;
   }
   double add_f32_f64(float x, double y)
   {
       return x + y;
   }
   UEXECUTER_DEFINE_SERVICE(my_uexecuter_service){
       // 兼容C的方法，手动提供返回值类型和参数类型
       UEXECUTER_FUNCTION_PROTOTYPE(add_f32_f64_i32, "add_f32_f64_i32", UEXECUTER_CALLER_TYPE_F64,
                                    UEXECUTER_CALLER_TYPE_F32, UEXECUTER_CALLER_TYPE_F64, UEXECUTER_CALLER_TYPE_I32), 
       // 仅C++可用的方法，使用 AUTO 来自动检测函数原型
       UEXECUTER_FUNCTION_PROTOTYPE_AUTO(add_f32_f64, "add_f32_f64"),
   };
   ```
2. Init the uexecuter system with your service: 
   ```cpp
   static uexecuter_t uexecuter;
   uexecuter_init(&uexecuter, my_uexecuter_service, UEXECUTER_SERVICE_N_FUNC(my_uexecuter_service));
   ```
3. Configure your transport system and bind your uexecuter to it: 
   ```cpp
   static uexecuter_transport_stm32_t uexecuter_transport;
   uexecuter_transport_stm32_init(&uexecuter_transport, USART1);
   uexecuter_bind(&uexecuter, &uexecuter_transport.base);
   uexecuter_transport_stm32_begin(&uexecuter_transport);
   extern "C" void usart1_irq()
   {
       uexecuter_transport_stm32_handle_isr(&uexecuter_transport);
   }
   extern "C" void USART1_IRQHandler(void) // 由 STM32CubeMX 生成，通常在 stm32f4xx_it.c 文件中
   {
       return usart1_irq();
   }
   ```

## LICENSE
Licensed under [MPL-2.0](./LICENSE)