/**
 ******************************************************************************
 * مثال ۳ — ADC + Biquad + FFT (تحلیل طیفی با CMSIS-DSP)
 *
 * جریان: ADC+DMA (8kHz) → DC Blocker → Biquad Band-Pass → بافر 1024
 * → FFT (arm_cfft_f32 از CMSIS-DSP) → Magnitude → (نمایش/ارسال)
 *
 * این مثال ترکیب کتابخانه با CMSIS-DSP را نشان می‌دهد: پردازش زمان با
 * dsp_* و تحلیل فرکانس با arm_cfft_f32.
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "arm_math.h"
#include "dsp.h"

#define FFT_SIZE  1024
#define BLOCK     128

static DSP_PLACE_SRAM1 volatile uint16_t adc_buf[BLOCK];
static DSP_PLACE_DTCM dsp_f32_t signal[FFT_SIZE];
static DSP_PLACE_DTCM dsp_f32_t fft_in[FFT_SIZE];   /* complex interleaved */
static DSP_PLACE_DTCM dsp_f32_t fft_mag[FFT_SIZE/2];

static dsp_dcblock_f32_t dc;
static dsp_biquad_f32_t bp;
static arm_cfft_instance_f32 fft_inst;
static volatile uint8_t dma_done = 0;
static volatile uint32_t sample_count = 0;

int main(void)
{
    HAL_Init();
    /* ... clock (8kHz ADC) / ADC / DMA init ... */

    /* DC Blocker + Biquad Band-Pass 300Hz..3.4kHz (صدای گفتار) */
    dsp_dcblock_init_f32(&dc, 0.995f);
    dsp_biquad_init_f32(&bp, DSP_TDF2);
    dsp_biquad_set_params_f32(&bp, DSP_BIQUAD_BPF, 8000, 1000, 1.0f, 0.0f);

    /* آماده‌سازی FFT — یک بار در Init */
    if (arm_cfft_init_f32(&fft_inst, FFT_SIZE) != ARM_MATH_SUCCESS) Error_Handler();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK);

    for (;;) {
        if (dma_done) {
            dma_done = 0;
            for (int i = 0; i < BLOCK; i++) {
                dsp_f32_t x = ((dsp_f32_t)adc_buf[i] - 2048.0f) / 2048.0f;
                dsp_dcblock_process_f32(&dc, x, &x);       /* حذف DC */
                dsp_biquad_process_sample_f32(&bp, x, &x); /* باندگذر */
                signal[sample_count % FFT_SIZE] = x;        /* جمع‌آوری */
                sample_count++;
            }
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK);
        }
        if (sample_count >= FFT_SIZE) {
            /* تحلیل FFT — خارج از مسیر Real-Time نمونه‌برداری */
            for (int i = 0; i < FFT_SIZE; i++) {
                fft_in[2*i]   = signal[i];
                fft_in[2*i+1] = 0.0f;
            }
            arm_cfft_f32(&fft_inst, fft_in, 0, 1);
            arm_cmplx_mag_f32(fft_in, fft_mag, FFT_SIZE/2);
            /* fft_mag اکنون طیف است — bin k ↔ فرکانس k·8000/1024 هرتز */
            sample_count = 0;
        }
    }
}
