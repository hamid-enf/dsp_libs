/**
 ******************************************************************************
 * مثال ۱ — ADC + DMA + FIR (فیلتر Low-Pass 100 هرتز روی نمونه‌های 1kHz)
 *
 * جریان: ADC1 (دوره‌ای، 1kHz) → DMA → بافر 128 → FIR (طراحی‌شده با
 * پنجره‌ی sinc) → خروجی در بافر دوم → (مثلاً) ارسال سریال.
 *
 * نکته‌ی STM32H7: بافر DMA باید در SRAM1..3 یا AXI-SRAM باشد (DTCM در دسترس
 * DMA نیست!). بافر پردازش CPU می‌تواند در DTCM باشد.
 *
 * Build: STM32CubeH7 + arm-none-eabi-gcc -O3 -mcpu=cortex-m7 -mfloat-abi=hard
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

/* --- بافرها --- */
#define BLOCK_SIZE 128
#define TAPS       48

/* بافر ADC/DMA — در SRAM (قابل دسترسی DMA) */
static DSP_PLACE_SRAM1 volatile uint16_t adc_buf[BLOCK_SIZE];
/* ضرایب FIR در Flash (const) */
static const dsp_f32_t fir_coeffs[TAPS] = {
    /* خروجی dsp_resample_design_lp_f32 یا طراحی Offline — برای 100Hz@1kHz */
    0.0001f, 0.0005f, 0.0013f, 0.0028f, 0.0052f, 0.0087f, 0.0135f, 0.0196f,
    0.0270f, 0.0355f, 0.0449f, 0.0548f, 0.0647f, 0.0740f, 0.0822f, 0.0888f,
    0.0934f, 0.0958f, 0.0960f, 0.0939f, 0.0898f, 0.0840f, 0.0768f, 0.0687f,
    0.0601f, 0.0513f, 0.0428f, 0.0348f, 0.0276f, 0.0214f, 0.0162f, 0.0119f,
    0.0085f, 0.0059f, 0.0039f, 0.0025f, 0.0015f, 0.0008f, 0.0004f, 0.0002f,
    0.0001f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f
};
/* حالت FIR — در DTCM (فقط CPU) */
static DSP_PLACE_DTCM dsp_f32_t fir_state[TAPS + BLOCK_SIZE - 1];
static dsp_fir_f32_t fir;

/* خروجی (CPU) */
static DSP_PLACE_DTCM dsp_f32_t out_f32[BLOCK_SIZE];
static volatile uint8_t dma_done = 0;

/* تبدیل 12bit ADC به float نرمال‌شده ±1 */
static inline dsp_f32_t adc_to_float(uint16_t v)
{
    return ((dsp_f32_t)v - 2048.0f) / 2048.0f;
}

void DMA_IRQHandler(void)
{
    if (DMA1_Stream0->ISR & DMA_FLAG_TCIF0) {
        DMA1_Stream0->IFCR = DMA_FLAG_TCIF0;
        dma_done = 1;   /* در ISR فقط پرچم — پردازش در حلقه‌ی اصلی */
    }
}

int main(void)
{
    HAL_Init();
    /* ... SystemClock_Config() ، MX_ADC1_Init() ، MX_DMA_Init() ... */

    /* راه‌اندازی FIR — API ساده */
    if (dsp_fir_init_f32(&fir, fir_coeffs, TAPS, BLOCK_SIZE,
                         DSP_FORM_DIRECT, fir_state, sizeof(fir_state)/4, 0) != DSP_OK) {
        Error_Handler();
    }

    /* شروع ADC دوره‌ای با DMA — بافر در SRAM (نه DTCM!) */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK_SIZE);

    for (;;) {
        if (dma_done) {
            dma_done = 0;
            /* تبدیل و فیلتر — یک بلاک */
            {
                dsp_f32_t in[BLOCK_SIZE];
                for (int i = 0; i < BLOCK_SIZE; i++) in[i] = adc_to_float(adc_buf[i]);
                dsp_fir_process_block_f32(&fir, in, out_f32, BLOCK_SIZE);
                /* out_f32 اکنون سیگنال 100Hz-فیلترشده است */
            }
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK_SIZE);  /* بلاک بعد */
        }
    }
}
