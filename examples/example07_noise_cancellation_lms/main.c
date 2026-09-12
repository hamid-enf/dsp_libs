/**
 ******************************************************************************
 * مثال ۷ — حذف نویز تطبیقی با LMS (Noise Cancellation)
 *
 * سناریو: میکروفون اصلی (d = گفتار + نویز) و میکروفون مرجع (x ≈ نویز).
 * LMS ضرایب را طوری تنظیم می‌کند که خروجی y ≈ نویزِ ورودی اصلی شود؛
 * e = d − y ≈ گفتار تمیز.
 *
 * نکات:
 *  - هم‌ترازی تأخیر دو میکروفون مهم است.
 *  - گام یادگیری μ کوچک (پایدارتر) در برابر همگرایی کندتر.
 *  - ورودی‌ها را قبل از LMS به ±0.5 مقیاس کنید تا از ناپایداری جلوگیری شود.
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

#define TAP_LEN 64
#define BLOCK   128

static DSP_PLACE_DTCM dsp_f32_t w[TAP_LEN];
static DSP_PLACE_DTCM dsp_f32_t xl[TAP_LEN];
static dsp_lms_f32_t lms;
static DSP_PLACE_SRAM1 int16_t mic_main[BLOCK];   /* d */
static DSP_PLACE_SRAM1 int16_t mic_ref[BLOCK];    /* x */
static DSP_PLACE_DTCM dsp_f32_t out[BLOCK];

int main(void)
{
    HAL_Init();
    /* ... SAI × 2 میکروفون / DMA init ... */

    /* μ = 0.005 برای TAP_LEN=64 — از ناپایداری μ > 1/(λ_max) جلوگیری کنید */
    if (dsp_lms_init_f32(&lms, TAP_LEN, 0.005f, 0.0f, w, xl) != DSP_OK) Error_Handler();

    for (;;) {
        /* (منتظر بلاک کامل دو میکروفون) */
        for (int i = 0; i < BLOCK; i++) {
            dsp_f32_t x = (dsp_f32_t)mic_ref[i]  / 65536.0f;   /* مرجع نویز */
            dsp_f32_t d = (dsp_f32_t)mic_main[i] / 65536.0f;   /* اصلی */
            dsp_f32_t y, e;
            dsp_lms_process_f32(&lms, x, d, &y, &e);
            out[i] = e;   /* سیگنال تمیز */
        }
        /* out → DAC/SAI برای گوش دادن یا ضبط */
    }
}
