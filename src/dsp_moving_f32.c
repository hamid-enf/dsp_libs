/**
 * @file dsp_moving_f32.c — SMA / WMA / EMA
 */
#include "dsp_moving.h"

#if DSP_ENABLE_MOVING_AVERAGE

/* --- SMA: O(1) با Running Sum --- */
dsp_err_t dsp_sma_init_f32(dsp_sma_f32_t *p, uint16_t window, dsp_f32_t *buf)
{
    if (p == NULL || buf == NULL) return DSP_ERR_NULL_PTR;
    if (window == 0) return DSP_ERR_INVALID_PARAMETER;
    p->window = window;
    p->pBuf = buf;
    p->sum = 0.0f;
    p->idx = 0;
    p->count = 0;
    return DSP_OK;
}

dsp_err_t dsp_sma_process_f32(dsp_sma_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    if (p->count >= p->window) {
        p->sum -= p->pBuf[p->idx];          /* حذف قدیمی‌ترین */
    } else {
        p->count++;
    }
    p->pBuf[p->idx] = x;
    p->sum += x;
    p->idx++;
    if (p->idx >= p->window) p->idx = 0;
    *y = p->sum / (dsp_f32_t)p->count;
    return DSP_OK;
}

dsp_err_t dsp_sma_process_block_f32(dsp_sma_f32_t *p, const dsp_f32_t *src,
                                    dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        dsp_err_t e = dsp_sma_process_f32(p, src[i], &dst[i]);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

dsp_err_t dsp_sma_reset_f32(dsp_sma_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->sum = 0.0f; p->idx = 0; p->count = 0;
    return DSP_OK;
}

/* --- WMA --- */
dsp_err_t dsp_wma_init_f32(dsp_wma_f32_t *p, uint16_t window, const dsp_f32_t *weights,
                           dsp_f32_t *buf)
{
    uint16_t i;
    if (p == NULL || weights == NULL || buf == NULL) return DSP_ERR_NULL_PTR;
    if (window == 0) return DSP_ERR_INVALID_PARAMETER;
    p->window = window;
    p->pW = weights;
    p->pBuf = buf;
    p->idx = 0;
    p->wsum = 0.0f;
    for (i = 0; i < window; i++) p->wsum += weights[i];
    if (p->wsum == 0.0f) p->wsum = 1.0f;
    return DSP_OK;
}

dsp_err_t dsp_wma_process_f32(dsp_wma_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    uint16_t i;
    dsp_f32_t acc = 0.0f;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    p->pBuf[p->idx] = x;
    for (i = 0; i < p->window; i++) {
        int32_t k = (int32_t)p->idx - (int32_t)i;
        if (k < 0) k += p->window;
        acc += p->pW[i] * p->pBuf[k];
    }
    p->idx++;
    if (p->idx >= p->window) p->idx = 0;
    *y = acc / p->wsum;
    return DSP_OK;
}

dsp_err_t dsp_wma_reset_f32(dsp_wma_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    return DSP_OK;
}

/* --- EMA --- */
dsp_err_t dsp_ema_init_f32(dsp_ema_f32_t *p, dsp_f32_t alpha)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (alpha <= 0.0f || alpha > 1.0f) return DSP_ERR_INVALID_PARAMETER;
    p->alpha = alpha;
    p->y = 0.0f;
    return DSP_OK;
}

dsp_err_t dsp_ema_set_alpha_f32(dsp_ema_f32_t *p, dsp_f32_t alpha)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (alpha <= 0.0f || alpha > 1.0f) return DSP_ERR_INVALID_PARAMETER;
    p->alpha = alpha;
    return DSP_OK;
}

dsp_err_t dsp_ema_process_f32(dsp_ema_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    p->y = p->alpha * x + (1.0f - p->alpha) * p->y;
    *y = p->y;
    return DSP_OK;
}

dsp_err_t dsp_ema_reset_f32(dsp_ema_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->y = 0.0f;
    return DSP_OK;
}

#endif /* DSP_ENABLE_MOVING_AVERAGE */
