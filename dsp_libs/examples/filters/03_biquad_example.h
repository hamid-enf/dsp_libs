/*
 * 03_biquad_example.h — اکولایزر سه‌باند استریو برای PCM16 interleaved
 *
 * کاربرد: صدا، اکولایزر، tone control و crossover سبک. برای هر کانال یک
 * instance جدا لازم است؛ state چپ و راست را هیچ‌وقت مشترک نکنید.
 * مسیر هر کانال: Low-Shelf -> Peaking -> High-Shelf.
 *
 * قرارداد تابع صوتی: pcm شامل [L,R,L,R,...] و frames تعداد فریم‌های استریو است.
 */
#ifndef DSP_EXAMPLE_03_BIQUAD_H
#define DSP_EXAMPLE_03_BIQUAD_H

#include "dsp.h"

#if DSP_ENABLE_BIQUAD

typedef struct {
    dsp_biquad_f32_t low[2];
    dsp_biquad_f32_t mid[2];
    dsp_biquad_f32_t high[2];
} dsp_example_biquad_eq_t;

static dsp_example_biquad_eq_t dsp_example_biquad_eq;

static inline dsp_err_t dsp_example_biquad_eq_init(uint32_t sample_rate)
{
    uint8_t ch;
    dsp_err_t e;
    if (sample_rate == 0u) return DSP_ERR_INVALID_PARAMETER;

    for (ch = 0u; ch < 2u; ++ch) {
        e = dsp_biquad_init_f32(&dsp_example_biquad_eq.low[ch], DSP_TDF2);
        if (e != DSP_OK) return e;
        e = dsp_biquad_init_f32(&dsp_example_biquad_eq.mid[ch], DSP_TDF2);
        if (e != DSP_OK) return e;
        e = dsp_biquad_init_f32(&dsp_example_biquad_eq.high[ch], DSP_TDF2);
        if (e != DSP_OK) return e;

        /* +2dB زیر 180Hz، +3dB در 1.2kHz، -2dB بالای 8kHz. */
        e = dsp_biquad_set_params_f32(&dsp_example_biquad_eq.low[ch],
                                      DSP_BIQUAD_LSHELF, sample_rate,
                                      180.0f, 0.707f, 2.0f);
        if (e != DSP_OK) return e;
        e = dsp_biquad_set_params_f32(&dsp_example_biquad_eq.mid[ch],
                                      DSP_BIQUAD_PEAK, sample_rate,
                                      1200.0f, 1.0f, 3.0f);
        if (e != DSP_OK) return e;
        e = dsp_biquad_set_params_f32(&dsp_example_biquad_eq.high[ch],
                                      DSP_BIQUAD_HSHELF, sample_rate,
                                      8000.0f, 0.707f, -2.0f);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

/* این تابع را بعد از init، مستقیماً داخل callback خروجی صدا صدا بزنید. */
static inline dsp_err_t dsp_example_biquad_eq_process_pcm16(int16_t *pcm,
                                                             uint16_t frames)
{
    uint16_t i;
    if (pcm == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0u; i < frames; ++i) {
        dsp_f32_t x, y;
        x = (dsp_f32_t)pcm[2u * i] / 32768.0f;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.low[0], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.mid[0], y, &x) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.high[0], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        pcm[2u * i] = (int16_t)dsp_clamp_f32(y * 32768.0f, -32768.0f, 32767.0f);

        x = (dsp_f32_t)pcm[2u * i + 1u] / 32768.0f;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.low[1], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.mid[1], y, &x) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_biquad_process_sample_f32(&dsp_example_biquad_eq.high[1], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        pcm[2u * i + 1u] = (int16_t)dsp_clamp_f32(y * 32768.0f, -32768.0f, 32767.0f);
    }
    return DSP_OK;
}
#endif /* DSP_ENABLE_BIQUAD */
#endif /* DSP_EXAMPLE_03_BIQUAD_H */
