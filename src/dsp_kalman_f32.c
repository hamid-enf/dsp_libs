/**
 * @file dsp_kalman_f32.c — فیلتر کالمن 1D
 *
 * مدل دینامیک ثابت:  x_k = x_{k-1} + w ،  z_k = x_k + v
 * پیش‌بینی: x̂⁻ = x̂ ، P⁻ = P + Q
 * به‌روزرسانی: K = P⁻/(P⁻+R) ، x̂ = x̂⁻ + K(z−x̂⁻) ، P = (1−K)·P⁻
 */
#include "dsp_kalman.h"

#if DSP_ENABLE_KALMAN

dsp_err_t dsp_kalman1d_init_f32(dsp_kalman1d_f32_t *p, dsp_f32_t x0,
                                dsp_f32_t p0, dsp_f32_t q, dsp_f32_t r)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (q < 0.0f || r < 0.0f || p0 < 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->q = q;
    p->r = r;
    p->x = x0;
    p->p = p0;
    p->k = 0.0f;
    return DSP_OK;
}

dsp_err_t dsp_kalman1d_set_noise_f32(dsp_kalman1d_f32_t *p, dsp_f32_t q, dsp_f32_t r)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (q < 0.0f || r < 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->q = q;
    p->r = r;
    return DSP_OK;
}

dsp_err_t dsp_kalman1d_predict_f32(dsp_kalman1d_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->p += p->q;   /* x بدون تغییر (دینامیک ثابت) */
    return DSP_OK;
}

dsp_err_t dsp_kalman1d_update_f32(dsp_kalman1d_f32_t *p, dsp_f32_t z, dsp_f32_t *xout)
{
    dsp_f32_t k, resid;
    if (p == NULL || xout == NULL) return DSP_ERR_NULL_PTR;
    k = p->p / (p->p + p->r);
    resid = z - p->x;
    p->x = p->x + k * resid;
    p->p = (1.0f - k) * p->p;
    p->k = k;
    *xout = p->x;
    return DSP_OK;
}

dsp_err_t dsp_kalman1d_process_f32(dsp_kalman1d_f32_t *p, dsp_f32_t z, dsp_f32_t *xout)
{
    /* پیش‌بینی + به‌روزرسانی (مدل ثابت) */
    dsp_kalman1d_predict_f32(p);
    return dsp_kalman1d_update_f32(p, z, xout);
}

dsp_err_t dsp_kalman1d_reset_f32(dsp_kalman1d_f32_t *p, dsp_f32_t x0, dsp_f32_t p0)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (p0 < 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->x = x0;
    p->p = p0;
    p->k = 0.0f;
    return DSP_OK;
}

#endif /* DSP_ENABLE_KALMAN */
