/**
 * @file dsp_savgol.c — Savitzky-Golay: طراحی ضرایب + پردازش
 *
 * طراحی (float64): ماتریس وندرموند A به‌ابعاد (2M+1)×(P+1) برای نقاط
 * یکنواخت -M..M. ضرایب SG برای مشتق مرتبه‌ی d:
 *   g = A · (AᵀA)⁻¹ · e_d      (e_d: ستون d-ام ماتریس واحد)
 * با حل سیستم نرمال AᵀA·β = e_d به روش حذف گاوسی و سپس g = A·β.
 */
#include "dsp_savgol.h"
#include "dsp_port.h"
#include <math.h>

#if DSP_ENABLE_SAVGOL

/* حذف گاوسی با Pivot جزئی برای سیستم n×n */
static int gauss_solve(double *A, double *b, int n)
{
    int i, j, k;
    for (k = 0; k < n; k++) {
        int piv = k;
        double maxv = fabs(A[k * n + k]);
        for (i = k + 1; i < n; i++) {
            if (fabs(A[i * n + k]) > maxv) { maxv = fabs(A[i * n + k]); piv = i; }
        }
        if (maxv < 1e-300) return -1;
        if (piv != k) {
            for (j = k; j < n; j++) { double t = A[k * n + j]; A[k * n + j] = A[piv * n + j]; A[piv * n + j] = t; }
            { double t = b[k]; b[k] = b[piv]; b[piv] = t; }
        }
        for (i = k + 1; i < n; i++) {
            double f = A[i * n + k] / A[k * n + k];
            for (j = k; j < n; j++) A[i * n + j] -= f * A[k * n + j];
            b[i] -= f * b[k];
        }
    }
    for (k = n - 1; k >= 0; k--) {
        double s = b[k];
        for (j = k + 1; j < n; j++) s -= A[k * n + j] * b[j];
        b[k] = s / A[k * n + k];
    }
    return 0;
}

dsp_err_t dsp_savgol_design_f32(uint16_t m, uint16_t polyOrder, uint16_t deriv,
                                dsp_f32_t *coeffsOut)
{
    int npts = 2 * (int)m + 1;
    int p = (int)polyOrder;
    int d = (int)deriv;
    double A[DSP_MAX_SG_WINDOW * (DSP_MAX_SG_WINDOW + 1)];
    double AtA[(DSP_MAX_SG_WINDOW + 1) * (DSP_MAX_SG_WINDOW + 1)];
    double rhs[DSP_MAX_SG_WINDOW + 1];
    int i, j, k;

    if (coeffsOut == NULL) return DSP_ERR_NULL_PTR;
    if (m == 0 || m > DSP_MAX_SG_WINDOW / 2) return DSP_ERR_INVALID_PARAMETER;
    if (p < 0 || p >= npts) return DSP_ERR_INVALID_PARAMETER;
    if (d < 0 || d > p) return DSP_ERR_INVALID_PARAMETER;

    /* A[i][j] = x_i^j  ، x_i = -m..m */
    for (i = 0; i < npts; i++) {
        double xi = (double)i - (double)m;
        double xp = 1.0;
        for (j = 0; j <= p; j++) {
            A[i * (p + 1) + j] = xp;
            xp *= xi;
        }
    }
    /* AᵀA */
    for (j = 0; j <= p; j++) {
        for (k = 0; k <= p; k++) {
            double s = 0.0;
            for (i = 0; i < npts; i++) s += A[i * (p + 1) + j] * A[i * (p + 1) + k];
            AtA[j * (p + 1) + k] = s;
        }
    }
    /* RHS = e_d */
    for (j = 0; j <= p; j++) rhs[j] = (j == d) ? 1.0 : 0.0;

    if (gauss_solve(AtA, rhs, p + 1) != 0) return DSP_ERR_DESIGN_CONVERGENCE;

    /* g = A·β  (ضرب در factorial(d) برای مشتق واقعی) */
    {
        double fact = 1.0;
        for (k = 2; k <= d; k++) fact *= (double)k;
        for (i = 0; i < npts; i++) {
            double s = 0.0;
            for (j = 0; j <= p; j++) s += A[i * (p + 1) + j] * rhs[j];
            coeffsOut[i] = (dsp_f32_t)(s * fact);
        }
    }
    return DSP_OK;
}

dsp_err_t dsp_savgol_init_f32(dsp_savgol_f32_t *p, uint16_t m,
                              const dsp_f32_t *coeffs, dsp_f32_t *buf)
{
    if (p == NULL || coeffs == NULL || buf == NULL) return DSP_ERR_NULL_PTR;
    if (m == 0 || m > DSP_MAX_SG_WINDOW / 2) return DSP_ERR_INVALID_PARAMETER;
    p->m = m;
    p->pCoeffs = coeffs;
    p->pBuf = buf;
    p->idx = 0;
    return DSP_OK;
}

dsp_err_t dsp_savgol_process_f32(dsp_savgol_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    uint16_t w, k;
    int32_t i0;
    dsp_f32_t acc = 0.0f;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    w = 2 * p->m + 1;
    p->pBuf[p->idx] = x;
    i0 = (int32_t)p->idx;
    /* پنجره‌ی مرکزی با تأخیر m: خروجی در لحظه‌ی n به ورودی‌های
       x[n-2m .. n] بستگی دارد (مرکز پنجره = x[n-m]) */
    for (k = 0; k < w; k++) {
        int32_t idx = i0 - (int32_t)(w - 1) + (int32_t)k;
        if (idx < 0) idx += w;
        if (idx >= (int32_t)w) idx -= w;
        acc += p->pCoeffs[k] * p->pBuf[idx];
    }
    p->idx++;
    if (p->idx >= w) p->idx = 0;
    *y = acc;
    return DSP_OK;
}

dsp_err_t dsp_savgol_reset_f32(dsp_savgol_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    return DSP_OK;
}

#endif /* DSP_ENABLE_SAVGOL */
