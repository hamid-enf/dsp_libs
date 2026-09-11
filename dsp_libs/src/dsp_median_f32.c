/**
 * @file dsp_median_f32.c — فیلتر میانه (f32 + q15)
 * روش: درج مرتب در آرایه‌ی کمکی — O(W) در هر نمونه، بدون تخصیص.
 */
#include "dsp_median.h"

#if DSP_ENABLE_MEDIAN

dsp_err_t dsp_median_init_f32(dsp_median_f32_t *p, uint16_t window,
                              dsp_f32_t *buf, dsp_f32_t *sorted)
{
    if (p == NULL || buf == NULL || sorted == NULL) return DSP_ERR_NULL_PTR;
    if (window == 0 || (window & 1u) == 0) return DSP_ERR_INVALID_PARAMETER;
    if (window > DSP_MAX_MEDIAN_WINDOW) return DSP_ERR_INVALID_PARAMETER;
    p->window = window;
    p->pBuf = buf;
    p->pSorted = sorted;
    p->idx = 0;
    p->count = 0;
    return DSP_OK;
}

dsp_err_t dsp_median_process_f32(dsp_median_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    uint16_t i, j, w;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    w = p->window;

    /* جایگزینی در حلقه */
    p->pBuf[p->idx] = x;
    p->idx++;
    if (p->idx >= w) p->idx = 0;
    if (p->count < w) p->count++;

    if (p->count < w) {
        *y = 0.0f;   /* warm-up: خروجی معتبر نیست */
        return DSP_OK;
    }

    /* ساخت نسخه‌ی مرتب با درج (کپی از حلقه + insertion sort) */
    for (i = 0; i < w; i++) {
        dsp_f32_t v = p->pBuf[(p->idx + i) % w];
        j = i;
        while (j > 0 && p->pSorted[j - 1] > v) {
            p->pSorted[j] = p->pSorted[j - 1];
            j--;
        }
        p->pSorted[j] = v;
    }
    *y = p->pSorted[w >> 1];
    return DSP_OK;
}

dsp_err_t dsp_median_process_block_f32(dsp_median_f32_t *p, const dsp_f32_t *src,
                                       dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        dsp_err_t e = dsp_median_process_f32(p, src[i], &dst[i]);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

dsp_err_t dsp_median_reset_f32(dsp_median_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0; p->count = 0;
    return DSP_OK;
}

/* --- Q15 --- */
dsp_err_t dsp_median_init_q15(dsp_median_q15_t *p, uint16_t window,
                              dsp_q15_t *buf, dsp_q15_t *sorted)
{
    if (p == NULL || buf == NULL || sorted == NULL) return DSP_ERR_NULL_PTR;
    if (window == 0 || (window & 1u) == 0) return DSP_ERR_INVALID_PARAMETER;
    if (window > DSP_MAX_MEDIAN_WINDOW) return DSP_ERR_INVALID_PARAMETER;
    p->window = window;
    p->pBuf = buf;
    p->pSorted = sorted;
    p->idx = 0;
    p->count = 0;
    return DSP_OK;
}

dsp_err_t dsp_median_process_q15(dsp_median_q15_t *p, dsp_q15_t x, dsp_q15_t *y)
{
    uint16_t i, j, w;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    w = p->window;
    p->pBuf[p->idx] = x;
    p->idx++;
    if (p->idx >= w) p->idx = 0;
    if (p->count < w) p->count++;
    if (p->count < w) { *y = 0; return DSP_OK; }

    for (i = 0; i < w; i++) {
        dsp_q15_t v = p->pBuf[(p->idx + i) % w];
        j = i;
        while (j > 0 && p->pSorted[j - 1] > v) {
            p->pSorted[j] = p->pSorted[j - 1];
            j--;
        }
        p->pSorted[j] = v;
    }
    *y = p->pSorted[w >> 1];
    return DSP_OK;
}

dsp_err_t dsp_median_reset_q15(dsp_median_q15_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0; p->count = 0;
    return DSP_OK;
}

#endif /* DSP_ENABLE_MEDIAN */
