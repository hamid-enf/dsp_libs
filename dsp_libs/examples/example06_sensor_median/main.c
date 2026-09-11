/**
 ******************************************************************************
 * مثال ۶ — حسگر + Median (حذف Spike/Impulse از داده‌ی ADC)
 *
 * جریان: ADC (مثلاً سنسور فاصله/جریان) → Median پنجره 5 → مقدار تمیز
 * برای کنترلر. Median در برابر Outlier مقاوم است (برخلاف میانگین).
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

#define MED_WINDOW 5

static dsp_q15_t med_buf[MED_WINDOW];
static dsp_q15_t med_sorted[MED_WINDOW];
static dsp_median_q15_t med;
static dsp_q15_t cleaned = 0;

int main(void)
{
    HAL_Init();
    /* ... ADC init ... */

    /* نسخه‌ی Q15 — مستقیم با داده‌ی خام ADC 12-bit (با 4 بیت شیفت چپ) */
    if (dsp_median_init_q15(&med, MED_WINDOW, med_buf, med_sorted) != DSP_OK) {
        Error_Handler();
    }

    for (;;) {
        uint16_t raw = (uint16_t)HAL_ADC_GetValue(&hadc1);
        dsp_q15_t x = (dsp_q15_t)(((uint32_t)raw) << 4);   /* Q15 */
        dsp_median_process_q15(&med, x, &cleaned);
        /* cleaned = خوانش بدون Spike */
        HAL_Delay(5);
    }
}
