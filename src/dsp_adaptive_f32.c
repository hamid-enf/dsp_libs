/**
 * @file dsp_adaptive_f32.c — LMS / NLMS / RLS
 */
#include "dsp_adaptive.h"
#include "dsp_port.h"

#if DSP_ENABLE_LMS || DSP_ENABLE_NLMS || DSP_ENABLE_RLS

/* ============ LMS ============ */
dsp_err_t dsp_lms_init_f32(dsp_lms_f32_t *p, uint16_t len, dsp_f32_t mu,
                           dsp_f32_t leakage, dsp_f32_t *weights, dsp_f32_t *delayline)
{
    if (p == NULL || weights == NULL || delayline == NULL) return DSP_ERR_NULL_PTR;
    if (len == 0 || mu <= 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->len = len;
    p->mu = mu;
    p->leakage = leakage;
    p->pW = weights;
    p->pX = delayline;
    p->idx = 0;
    return DSP_OK;
}

dsp_err_t dsp_lms_process_f32(dsp_lms_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                              dsp_f32_t *y, dsp_f32_t *e)
{
    uint16_t i, len;
    dsp_f32_t yhat = 0.0f, err;
    if (p == NULL || y == NULL || e == NULL) return DSP_ERR_NULL_PTR;
    len = p->len;

    p->pX[p->idx] = x;
    for (i = 0; i < len; i++) {
        int32_t k = (int32_t)p->idx - (int32_t)i;
        if (k < 0) k += len;
        yhat += p->pW[i] * p->pX[k];
    }
    err = d - yhat;
    for (i = 0; i < len; i++) {
        int32_t k = (int32_t)p->idx - (int32_t)i;
        if (k < 0) k += len;
        /* w_i ← (1 − μ·leakage)·w_i + μ·e·x_i */
        if (p->leakage > 0.0f) {
            p->pW[i] = (1.0f - p->mu * p->leakage) * p->pW[i] + p->mu * err * p->pX[k];
        } else {
            p->pW[i] += p->mu * err * p->pX[k];
        }
    }
    p->idx++;
    if (p->idx >= len) p->idx = 0;
    *y = yhat;
    *e = err;
    return DSP_OK;
}

dsp_err_t dsp_lms_set_mu_f32(dsp_lms_f32_t *p, dsp_f32_t mu)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (mu <= 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->mu = mu;
    return DSP_OK;
}

dsp_err_t dsp_lms_reset_f32(dsp_lms_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    return DSP_OK;
}

/* ============ NLMS ============ */
dsp_err_t dsp_nlms_init_f32(dsp_nlms_f32_t *p, uint16_t len, dsp_f32_t mu,
                            dsp_f32_t delta, dsp_f32_t *weights, dsp_f32_t *delayline)
{
    if (p == NULL || weights == NULL || delayline == NULL) return DSP_ERR_NULL_PTR;
    if (len == 0 || mu <= 0.0f) return DSP_ERR_INVALID_PARAMETER;
    p->len = len;
    p->mu = mu;
    p->delta = delta;
    p->pW = weights;
    p->pX = delayline;
    p->idx = 0;
    return DSP_OK;
}

dsp_err_t dsp_nlms_process_f32(dsp_nlms_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                               dsp_f32_t *y, dsp_f32_t *e)
{
    uint16_t i, len;
    dsp_f32_t yhat = 0.0f, err, norm = p->delta, mu_eff;
    if (p == NULL || y == NULL || e == NULL) return DSP_ERR_NULL_PTR;
    len = p->len;

    p->pX[p->idx] = x;
    for (i = 0; i < len; i++) {
        int32_t k = (int32_t)p->idx - (int32_t)i;
        dsp_f32_t xv;
        if (k < 0) k += len;
        xv = p->pX[k];
        yhat += p->pW[i] * xv;
        norm += xv * xv;
    }
    err = d - yhat;
    mu_eff = p->mu / norm;
    for (i = 0; i < len; i++) {
        int32_t k = (int32_t)p->idx - (int32_t)i;
        if (k < 0) k += len;
        p->pW[i] += mu_eff * err * p->pX[k];
    }
    p->idx++;
    if (p->idx >= len) p->idx = 0;
    *y = yhat;
    *e = err;
    return DSP_OK;
}

dsp_err_t dsp_nlms_reset_f32(dsp_nlms_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    return DSP_OK;
}

/* ============ RLS ============ */
dsp_err_t dsp_rls_init_f32(dsp_rls_f32_t *p, uint16_t len, dsp_f32_t lambda,
                           dsp_f32_t delta, dsp_f32_t *weights, dsp_f32_t *delayline,
                           dsp_f32_t *cov)
{
    uint16_t i;
    if (p == NULL || weights == NULL || delayline == NULL || cov == NULL) {
        return DSP_ERR_NULL_PTR;
    }
    if (len == 0) return DSP_ERR_INVALID_PARAMETER;
    if (lambda <= 0.0f || lambda > 1.0f) return DSP_ERR_INVALID_PARAMETER;
    p->len = len;
    p->lambda = lambda;
    p->delta = delta;
    p->pW = weights;
    p->pX = delayline;
    p->pP = cov;
    p->idx = 0;
    for (i = 0; i < len; i++) {
        uint16_t j;
        for (j = 0; j < len; j++) {
            p->pP[i * len + j] = (i == j) ? delta : 0.0f;
        }
    }
    return DSP_OK;
}

dsp_err_t dsp_rls_process_f32(dsp_rls_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                              dsp_f32_t *y, dsp_f32_t *e)
{
    uint16_t i, j, len;
    dsp_f32_t yhat = 0.0f, err, den, k[DSP_MAX_ADAPTIVE_LEN];
    if (p == NULL || y == NULL || e == NULL) return DSP_ERR_NULL_PTR;
    len = p->len;
    if (len > DSP_MAX_ADAPTIVE_LEN) return DSP_ERR_INVALID_PARAMETER;

    p->pX[p->idx] = x;
    for (i = 0; i < len; i++) {
        int32_t ii = (int32_t)p->idx - (int32_t)i;
        if (ii < 0) ii += len;
        yhat += p->pW[i] * p->pX[ii];
    }
    err = d - yhat;

    /* k = P·x / (λ + xᵀ·P·x) */
    den = p->lambda;
    for (i = 0; i < len; i++) {
        int32_t ii = (int32_t)p->idx - (int32_t)i;
        dsp_f32_t s = 0.0f, xv;
        if (ii < 0) ii += len;
        xv = p->pX[ii];
        for (j = 0; j < len; j++) {
            int32_t jj = (int32_t)p->idx - (int32_t)j;
            if (jj < 0) jj += len;
            s += p->pP[i * len + j] * p->pX[jj];
        }
        k[i] = s;
        den += xv * s;
    }
    for (i = 0; i < len; i++) k[i] /= den;

    /* w ← w + k·e */
    for (i = 0; i < len; i++) p->pW[i] += k[i] * err;

    /* P ← (P − k·xᵀ·P)/λ
       ابتدا xP_j = Σ_t x_t·P_tj  سپس  P_ij −= k_i·xP_j */
    {
        dsp_f32_t xP[DSP_MAX_ADAPTIVE_LEN];
        for (j = 0; j < len; j++) {
            dsp_f32_t s = 0.0f;
            uint16_t t;
            for (t = 0; t < len; t++) {
                int32_t tt = (int32_t)p->idx - (int32_t)t;
                dsp_f32_t xt;
                if (tt < 0) tt += len;
                xt = p->pX[tt];
                s += xt * p->pP[t * len + j];
            }
            xP[j] = s;
        }
        for (i = 0; i < len; i++) {
            for (j = 0; j < len; j++) {
                p->pP[i * len + j] = (p->pP[i * len + j] - k[i] * xP[j]) / p->lambda;
            }
        }
    }

    p->idx++;
    if (p->idx >= len) p->idx = 0;
    *y = yhat;
    *e = err;
    return DSP_OK;
}

dsp_err_t dsp_rls_reset_f32(dsp_rls_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    return DSP_OK;
}

#endif /* any adaptive */
