/**
 ******************************************************************************
 * مثال ۴ — Audio I2S + Biquad (اکولایزر ۳-باند روی خروجی صدا)
 *
 * جریان: SAI/I2S (48kHz، Stereo) → DMA in → هر کانال: Biquad
 * (Low-Shelf / Peaking / High-Shelf) → SAI out.
 *
 * نکته: در حالت Stereo، کانال L/R به‌صورت Interleaved است؛ برای سرعت،
 * پردازش بلاک‌ای انجام می‌شود. بافرهای صوتی باید 32-بایت الاین باشند
 * (بهتر: Double-Buffer با HAL).
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

#define AUDIO_BLOCK 256    /* نمونه به ازای هر کانال */

/* بافرهای I2S (interleaved L/R) — SRAM برای DMA */
static DSP_PLACE_SRAM1 int16_t i2s_in[2 * AUDIO_BLOCK];
static DSP_PLACE_SRAM1 int16_t i2s_out[2 * AUDIO_BLOCK];

/* سه باند اکولایزر */
static dsp_biquad_f32_t eq_low, eq_mid, eq_high;
static DSP_PLACE_DTCM dsp_f32_t tmp[AUDIO_BLOCK];

int main(void)
{
    HAL_Init();
    /* ... SAI/I2S init 48kHz، DMA circular ... */

    /* اکولایزر: Low-Shelf +1dB@200Hz ، Peaking +3dB@1kHz ، High-Shelf −2dB@8kHz */
    dsp_biquad_init_f32(&eq_low,  DSP_TDF2);
    dsp_biquad_init_f32(&eq_mid,  DSP_TDF2);
    dsp_biquad_init_f32(&eq_high, DSP_TDF2);
    dsp_biquad_set_params_f32(&eq_low,  DSP_BIQUAD_LSHELF, 48000, 200, 0.707f,  1.0f);
    dsp_biquad_set_params_f32(&eq_mid,  DSP_BIQUAD_PEAK,   48000, 1000, 1.0f,    3.0f);
    dsp_biquad_set_params_f32(&eq_high, DSP_BIQUAD_HSHELF, 48000, 8000, 0.707f, -2.0f);

    HAL_I2S_Receive_DMA(&hi2s1, (uint16_t*)i2s_in, 2 * AUDIO_BLOCK);
    HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)i2s_out, 2 * AUDIO_BLOCK);

    for (;;) {
        /* (در HAL با Callback یا سِمافور، صبر کن تا بلاک کامل شود) */
        /* کانال L */
        for (int i = 0; i < AUDIO_BLOCK; i++) {
            tmp[i] = (dsp_f32_t)i2s_in[2*i] / 32768.0f;
            dsp_biquad_process_sample_f32(&eq_low,  tmp[i], &tmp[i]);
            dsp_biquad_process_sample_f32(&eq_mid,  tmp[i], &tmp[i]);
            dsp_biquad_process_sample_f32(&eq_high, tmp[i], &tmp[i]);
            i2s_out[2*i] = (int16_t)dsp_clamp_f32(tmp[i] * 32768.0f, -32768.0f, 32767.0f);
        }
        /* کانال R — همان زنجیره (یا یک اکولایزر جدا برای استریو) */
        for (int i = 0; i < AUDIO_BLOCK; i++) {
            tmp[i] = (dsp_f32_t)i2s_in[2*i+1] / 32768.0f;
            dsp_biquad_process_sample_f32(&eq_low,  tmp[i], &tmp[i]);
            dsp_biquad_process_sample_f32(&eq_mid,  tmp[i], &tmp[i]);
            dsp_biquad_process_sample_f32(&eq_high, tmp[i], &tmp[i]);
            i2s_out[2*i+1] = (int16_t)dsp_clamp_f32(tmp[i] * 32768.0f, -32768.0f, 32767.0f);
        }
    }
}
