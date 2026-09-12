/**
 ******************************************************************************
 * مثال ۵ — حسگر + Moving Average (نرم‌سازی خوانش دما/فشار)
 *
 * جریان: I2C/SPI حسگر → SMA با پنجره 16 (O(1) با Running Sum) → مقدار
 * نرم‌شده برای کنترلر.
 *
 * نکته: SMA تأخیر (W−1)/2 نمونه دارد؛ اگر تأخیر مهم است از EMA با
 * alpha مناسب استفاده کنید.
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

#define WINDOW 16

static dsp_f32_t sma_buf[WINDOW];
static dsp_sma_f32_t sma;
static dsp_f32_t filtered = 0.0f;

/* (الزاماً) خواندن خام حسگر — مثال: سنسور دما 12-bit */
static uint16_t sensor_read_raw(void) { return 0; /* HAL_I2C_Mem_Read(...) */ }

int main(void)
{
    HAL_Init();
    /* ... I2C init ... */

    if (dsp_sma_init_f32(&sma, WINDOW, sma_buf) != DSP_OK) Error_Handler();

    for (;;) {
        uint16_t raw = sensor_read_raw();
        dsp_f32_t x = (dsp_f32_t)raw / 4096.0f;     /* نرمال‌سازی */
        dsp_sma_process_f32(&sma, x, &filtered);    /* O(1) */
        /* filtered = مقدار نرم‌شده — برای کنترل/نمایش */
        HAL_Delay(10);
    }
}
