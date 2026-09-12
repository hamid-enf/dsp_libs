/**
 * @file dsp_notch_f32.c — Notch Filter (TDF2) با تنظیم Runtime
 */
#include "dsp_notch.h"
#include "dsp_port.h"

#if DSP_ENABLE_NOTCH

static void dsp_notch_calc(dsp_notch_f32_t *p)
{
    dsp_f32_t w0 = DSP_TWO_PI_F * p->fn / (dsp_f32_t)p->fs;
    dsp_f32_t alpha = dsp_sin_f32(w0) / (2.0f * p->q);
    dsp_f32_t cosw0 = dsp_cos_f32(w0);
    p->b0 = 1.0f / (1.0f + alpha);
    p->b1 = -2.0f * cosw0 * p->b0;
    p->b2 = p->b0;
    p->a1 = -2.0f * cosw0 * p->b0;
    p->a2 = (1.0f - alpha) * p->b0;
}

dsp_err_t dsp_notch_init_f32(dsp_notch_f32_t *p, uint32_t fs, dsp_f32_t fn, dsp_f32_t q)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (fs == 0 || fn <= 0.0f || fn >= (dsp_f32_t)fs * 0.5f || q <= 0.0f) {
        return DSP_ERR_INVALID_PARAMETER;
    }
    p->fs = fs; p->fn = fn; p->q = q;
    p->d1 = 0.0f; p->d2 = 0.0f;
    dsp_notch_calc(p);
    return DSP_OK;
}

dsp_err_t dsp_notch_set_freq_f32(dsp_notch_f32_t *p, uint32_t fs, dsp_f32_t fn, dsp_f32_t q)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (fs == 0 || fn <= 0.0f || fn >= (dsp_f32_t)fs * 0.5f || q <= 0.0f) {
        return DSP_ERR_INVALID_PARAMETER;
    }
    p->fs = fs; p->fn = fn; p->q = q;
    dsp_notch_calc(p);
    return DSP_OK;
}

dsp_err_t dsp_notch_process_f32(dsp_notch_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    dsp_f32_t y0;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    y0 = p->b0 * x + p->d1;
    p->d1 = p->b1 * x - p->a1 * y0 + p->d2;
    p->d2 = p->b2 * x - p->a2 * y0;
    *y = y0;
    return DSP_OK;
}

dsp_err_t dsp_notch_process_block_f32(dsp_notch_f32_t *p, const dsp_f32_t *src,
                                      dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        dsp_err_t e = dsp_notch_process_f32(p, src[i], &dst[i]);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

dsp_err_t dsp_notch_reset_f32(dsp_notch_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->d1 = 0.0f; p->d2 = 0.0f;
    return DSP_OK;
}

#endif /* DSP_ENABLE_NOTCH */
