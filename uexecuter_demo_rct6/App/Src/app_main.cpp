#include "../Inc/app_main.h"
#include "global.h"
#include "oled.h"
#include "spi.h"
#include "uexecuter.h"
#include "uexecuter_service.h"
#include "uexecuter_transport_stm32.h"
#include "usart.h"
#include <cmath>
#include <memory>

static uexecuter_transport_stm32_t uexecuter_transport;
int32_t inc_i32(int32_t x)
{
    return x + 1;
}

int32_t add_i32_2(int32_t x)
{
    return x + 2;
}

uint32_t addr_of_add_i32_2()
{
    return (uint32_t)(&add_i32_2);
}

double add_f32_f64_i32(float x, double y, int32_t z)
{
    return x + y + z;
}

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    u8g2_DrawLine(&u8g2, x1, y1, x2, y2);
}

void draw_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    u8g2_DrawFrame(&u8g2, x, y, w, h);
}

void draw_circle(uint16_t x0, uint16_t y0, uint16_t rad)
{
    u8g2_DrawCircle(&u8g2, x0, y0, rad, U8G2_DRAW_ALL);
}

void draw_pixel(uint16_t x0, uint16_t y0)
{
    u8g2_DrawPixel(&u8g2, x0, y0);
}

void draw_str(uint16_t x, uint16_t y, const char *str)
{
    u8g2_DrawUTF8(&u8g2, x, y, str);
}

void clear_screen()
{
    u8g2_ClearBuffer(&u8g2);
}

UEXECUTER_DEFINE_SERVICE(my_uexecuter_service){
    UEXECUTER_FUNCTION_PROTOTYPE(inc_i32, "inc_i32", UEXECUTER_CALLER_TYPE_I32, UEXECUTER_CALLER_TYPE_I32),
    UEXECUTER_FUNCTION_PROTOTYPE(add_f32_f64_i32, "add_f32_f64_i32", UEXECUTER_CALLER_TYPE_F64,
                                 UEXECUTER_CALLER_TYPE_F32, UEXECUTER_CALLER_TYPE_F64, UEXECUTER_CALLER_TYPE_I32),
    UEXECUTER_FUNCTION_PROTOTYPE(addr_of_add_i32_2, "addr_of_add_i32_2", UEXECUTER_CALLER_TYPE_U32),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(draw_line, "draw_line"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(draw_circle, "draw_circle"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(draw_frame, "draw_frame"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(clear_screen, "clear_screen"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(draw_str, "draw_str"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(draw_pixel, "draw_pixel"),
    UEXECUTER_FUNCTION_PROTOTYPE_AUTO(HAL_Delay, "delay")};

static uexecuter_t uexecuter;

extern "C" void app_pre_init()
{
    // do nothing
}
extern "C" void app_init()
{
    // do nothing
}
extern "C" void app_sys_init()
{
    // do nothing
}

extern "C" void app_main()
{
    uexecuter_transport_stm32_init(&uexecuter_transport, USART1);
    uexecuter_init(&uexecuter, my_uexecuter_service, UEXECUTER_SERVICE_N_FUNC(my_uexecuter_service));
    uexecuter_bind(&uexecuter, &uexecuter_transport.base);
    uexecuter_transport_stm32_begin(&uexecuter_transport);
    oled_init();
    u8g2_ClearBuffer(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x13_mf);
    for (;;)
    {
        u8g2_SendBuffer(&u8g2);
        HAL_Delay(16);
    }
}

extern "C" void usart1_irq()
{
    uexecuter_transport_stm32_handle_isr(&uexecuter_transport);
}