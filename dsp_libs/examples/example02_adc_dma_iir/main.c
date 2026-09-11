/**
 ******************************************************************************
 * مثال ۲ — ADC + DMA + IIR (Butterworth مرتبه 4 طراحی‌شده در زمان اجرا)
 *
 * جریان: ADC+DMA (1kHz) → طراحی SOS با dsp_design_butterworth → پردازش
 * با dsp_iir (SOS/TDF2) → خروجی.
 *
 * این مثال «طراحی در زمان اجرا» را نشان می‌دهد؛ برای پروژه‌های واقعی
 * می‌توانید ضرایب را Offline تولید و در Flash ذخیره کنید (صرفه‌جویی در
 * RAM/Flash و زمان بوت).
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

#define BLOCK_SIZE 128
#define ORDER      4

static DSP_PLACE_SRAM1 volatile uint16_t adc_buf[BLOCK_SIZE];

/* خروجی طراحی (Design-Time) — RAM */
static double  sos_f64[ (ORDER/2) * 5 ];
static dsp_f32_t sos_f32[ (ORDER/2) * 5 ];
static dsp_f32_t iir_state[ (ORDER/2) * 2 ];
static DSP_PLACE_DTCM dsp_f32_t out_f32[BLOCK_SIZE];
static dsp_iir_f32_t iir;
static volatile uint8_t dma_done = 0;

int main(void)
{
    uint16_t nsec = 0;
    dsp_design_params_t cfg;

    HAL_Init();
    /* ... clock/ADC/DMA init ... */

    /* ۱) طراحی Butterworth LP مرتبه 4 — فرکانس قطع 100Hz در 1kHz */
    memset(&cfg, 0, sizeof(cfg));
    cfg.fs = 1000.0;
    cfg.fc = 100.0;
    cfg.type = DSP_FILTER_LP;
    cfg.order = ORDER;
    cfg.pSOS = sos_f64;
    cfg.pNumSections = &nsec;
    if (dsp_design_butterworth(&cfg) != DSP_OK) Error_Handler();

    /* ۲) تبدیل به float32 و راه‌اندازی IIR (SOS / TDF2) */
    dsp_design_sos_f64_to_f32(sos_f64, nsec, sos_f32);
    if (dsp_iir_init_f32(&iir, DSP_TDF2, ORDER, sos_f32, iir_state,
                         sizeof(iir_state)/4, 1) != DSP_OK) Error_Handler();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK_SIZE);

    for (;;) {
        if (dma_done) {
            dma_done = 0;
            for (int i = 0; i < BLOCK_SIZE; i++) {
                dsp_f32_t x = ((dsp_f32_t)adc_buf[i] - 2048.0f) / 2048.0f;
                dsp_iir_process_sample_f32(&iir, x, &out_f32[i]);
            }
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, BLOCK_SIZE);
        }
    }
}
