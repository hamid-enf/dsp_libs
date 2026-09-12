/**
 * @file dsp_dcblock_f32.c — DC Blocker:  y[n] = x[n] − x[n−1] + R·y[n−1]
 */
#include "dsp_dcblock.h"

#if DSP_ENABLE_DCBLOCK

dsp_err_t dsp_dcblock_init_f32(dsp_dcblock_f32_t *p, dsp_f32_t r)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (r < 0.0f || r >= 1.0f) return DSP_ERR_INVALID_PARAMETER;
    p->r = r;
    p->x1 = 0.0f;
    p->y1 = 0.0f;
    return DSP_OK;
}

dsp_err_t dsp_dcblock_process_f32(dsp_dcblock_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    dsp_f32_t out;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    out = x - p->x1 + p->r * p->y1;
    p->x1 = x;
    p->y1 = out;
    *y = out;
    return DSP_OK;
}

dsp_err_t dsp_dcblock_process_block_f32(dsp_dcblock_f32_t *p, const dsp_f32_t *src,
                                        dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        dsp_err_t e = dsp_dcblock_process_f32(p, src[i], &dst[i]);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

dsp_err_t dsp_dcblock_reset_f32(dsp_dcblock_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->x1 = 0.0f; p->y1 = 0.0f;
    return DSP_OK;
}

#endif /* DSP_ENABLE_DCBLOCK */
