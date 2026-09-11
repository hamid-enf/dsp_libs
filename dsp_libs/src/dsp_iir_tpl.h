/**
 ******************************************************************************
 * @file    dsp_iir_tpl.h  (internal — template)
 * @brief   پیاده‌سازی مشترک IIR برای دقت‌های مختلف
 *
 * فایل‌های includeکننده: dsp_iir_f32.c  dsp_iir_f64.c  dsp_iir_q15.c  dsp_iir_q31.c
 *
 * ماکروهای مورد نیاز:
 *   DSP_IIR_T_ENABLED / TYPE / ACC / SUFFIX / FLOAT
 *   DSP_IIR_TPL_OUT(acc, p) / SATADD / ACCUM(acc, expr)
 *   DSP_IIR_T_HAVE_SHIFT — 1 برای Fixed (فیلد shift در ساختار موجود است)
 ******************************************************************************
 */

#ifndef DSP_IIR_TPL_H
#define DSP_IIR_TPL_H

#if DSP_IIR_T_ENABLED

#include "dsp_iir.h"
#include "dsp_port.h"

#define DSP_IIR_TPL_CAT2(a, b)  a##b
#define DSP_IIR_TPL_CAT(a, b)   DSP_IIR_TPL_CAT2(a, b)
#define DSP_IIR_TPL_STRUCT      DSP_IIR_TPL_CAT(DSP_IIR_TPL_CAT(dsp_iir_, DSP_IIR_T_SUFFIX), _t)
#define DSP_IIR_TPL_FN(name)    DSP_IIR_TPL_CAT(DSP_IIR_TPL_CAT(dsp_iir_, DSP_IIR_TPL_CAT(name, _)), DSP_IIR_T_SUFFIX)

/* ---------------------------------------------------------------------------
 * Init
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_IIR_TPL_FN(init)(DSP_IIR_TPL_STRUCT *p, uint8_t form, uint16_t order,
                               const DSP_IIR_T_TYPE *coeffs
#if DSP_IIR_T_HAVE_SHIFT
                               , uint8_t shift
#endif
                               , DSP_IIR_T_TYPE *state,
                               uint32_t stateLen, uint8_t sos)
{
    uint32_t need;
    uint16_t stages = 0;

    if (p == NULL || coeffs == NULL || state == NULL) return DSP_ERR_NULL_PTR;
    if (order == 0) return DSP_ERR_INVALID_PARAMETER;
#if !DSP_IIR_T_FLOAT
    /* Fixed-Point: فقط DF1 (همان استدلال Biquad) */
    if (form != DSP_DF1) return DSP_ERR_UNSUPPORTED;
#endif
    if (sos) {
        if (form != DSP_TDF2 && form != DSP_DF1) return DSP_ERR_UNSUPPORTED;
        if (order & 1u) return DSP_ERR_INVALID_PARAMETER;   /* SOS به مرتبه‌ی زوج نیاز دارد */
        stages = order >> 1;
        need = (uint32_t)stages * ((form == DSP_DF1) ? 4u : 2u);
    } else {
        if (form != DSP_DF1 && form != DSP_DF2 && form != DSP_TDF2) {
            return DSP_ERR_INVALID_PARAMETER;
        }
        need = (form == DSP_DF1) ? (uint32_t)order * 2u : (uint32_t)order;
    }
    if (stateLen < need) return DSP_ERR_BUFFER_TOO_SMALL;

    p->form     = form;
    p->order    = order;
    p->stages   = stages;
    p->sos      = sos ? 1u : 0u;
    p->pCoeffs  = coeffs;
    p->pState   = state;
    p->stateLen = stateLen;
#if DSP_IIR_T_HAVE_SHIFT
    p->shift = shift;
#endif
    memset(state, 0, (size_t)need * sizeof(DSP_IIR_T_TYPE));
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * Reset
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_IIR_TPL_FN(reset)(DSP_IIR_TPL_STRUCT *p)
{
    uint32_t need;
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (p->pState == NULL) return DSP_ERR_NOT_INITIALIZED;
    need = p->sos ? ((p->form == DSP_DF1) ? (uint32_t)p->stages * 4u : (uint32_t)p->stages * 2u)
                  : ((p->form == DSP_DF1) ? (uint32_t)p->order * 2u : (uint32_t)p->order);
    memset(p->pState, 0, (size_t)need * sizeof(DSP_IIR_T_TYPE));
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * پردازش یک نمونه
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_IIR_TPL_FN(process_sample)(DSP_IIR_TPL_STRUCT *p, DSP_IIR_T_TYPE x,
                                         DSP_IIR_T_TYPE *y)
{
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;

    if (p->sos) {
        /* Cascade of SOS — TDF2 یا DF1 */
        uint16_t s;
        const DSP_IIR_T_TYPE *c = p->pCoeffs;
        DSP_IIR_T_TYPE *st = p->pState;
        if (p->form == DSP_TDF2) {
            for (s = 0; s < p->stages; s++) {
                DSP_IIR_T_ACC yacc = 0, nd1 = 0, nd2 = 0;
                DSP_IIR_T_TYPE y0;
                DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)c[0] * (DSP_IIR_T_ACC)x);
                DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)st[0]);
                y0 = DSP_IIR_TPL_OUT(yacc, p);
                DSP_IIR_TPL_ACCUM(nd1, (DSP_IIR_T_ACC)c[1] * (DSP_IIR_T_ACC)x);
                DSP_IIR_TPL_ACCUM(nd1, -(DSP_IIR_T_ACC)c[3] * (DSP_IIR_T_ACC)y0);
                DSP_IIR_TPL_ACCUM(nd1, (DSP_IIR_T_ACC)st[1]);
                st[0] = DSP_IIR_TPL_OUT(nd1, p);
                DSP_IIR_TPL_ACCUM(nd2, (DSP_IIR_T_ACC)c[2] * (DSP_IIR_T_ACC)x);
                DSP_IIR_TPL_ACCUM(nd2, -(DSP_IIR_T_ACC)c[4] * (DSP_IIR_T_ACC)y0);
                st[1] = DSP_IIR_TPL_OUT(nd2, p);
                x = y0;
                c += 5; st += 2;
            }
            *y = x;
            return DSP_OK;
        } else {
            for (s = 0; s < p->stages; s++) {
                DSP_IIR_T_ACC acc = 0;
                DSP_IIR_T_TYPE y0;
                DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)c[0] * (DSP_IIR_T_ACC)x);
                DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)c[1] * (DSP_IIR_T_ACC)st[0]);
                DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)c[2] * (DSP_IIR_T_ACC)st[1]);
                DSP_IIR_TPL_ACCUM(acc, -(DSP_IIR_T_ACC)c[3] * (DSP_IIR_T_ACC)st[2]);
                DSP_IIR_TPL_ACCUM(acc, -(DSP_IIR_T_ACC)c[4] * (DSP_IIR_T_ACC)st[3]);
                y0 = DSP_IIR_TPL_OUT(acc, p);
                st[1] = st[0]; st[0] = x;
                st[3] = st[2]; st[2] = y0;
                x = y0;
                c += 5; st += 4;
            }
            *y = x;
            return DSP_OK;
        }
    }

    if (p->form == DSP_DF1) {
        /* حالت: [x_{n-1}..x_{n-N} | y_{n-1}..y_{n-N}] */
        uint16_t N = p->order, k;
        const DSP_IIR_T_TYPE *b = p->pCoeffs;           /* b0..bN */
        const DSP_IIR_T_TYPE *a = p->pCoeffs + N + 1;   /* a1..aN */
        DSP_IIR_T_TYPE *xs = p->pState;
        DSP_IIR_T_TYPE *ys = p->pState + N;
        DSP_IIR_T_ACC acc = 0;
        DSP_IIR_T_TYPE y0;
        DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)b[0] * (DSP_IIR_T_ACC)x);
        for (k = 0; k < N; k++) {
            DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)b[k + 1] * (DSP_IIR_T_ACC)xs[k]);
        }
        for (k = 0; k < N; k++) {
            DSP_IIR_TPL_ACCUM(acc, -(DSP_IIR_T_ACC)a[k] * (DSP_IIR_T_ACC)ys[k]);
        }
        y0 = DSP_IIR_TPL_OUT(acc, p);
        for (k = N; k > 1; k--) { xs[k - 1] = xs[k - 2]; ys[k - 1] = ys[k - 2]; }
        if (N > 0) { xs[0] = x; ys[0] = y0; }
        *y = y0;
        return DSP_OK;
    }

    if (p->form == DSP_DF2) {
        uint16_t N = p->order, k;
        const DSP_IIR_T_TYPE *b = p->pCoeffs;
        const DSP_IIR_T_TYPE *a = p->pCoeffs + N + 1;
        DSP_IIR_T_TYPE *ws = p->pState;
        DSP_IIR_T_ACC wacc = 0, yacc = 0;
        DSP_IIR_T_TYPE w;
        DSP_IIR_TPL_ACCUM(wacc, (DSP_IIR_T_ACC)x);
        for (k = 0; k < N; k++) {
            DSP_IIR_TPL_ACCUM(wacc, -(DSP_IIR_T_ACC)a[k] * (DSP_IIR_T_ACC)ws[k]);
        }
        w = DSP_IIR_TPL_OUT(wacc, p);
        DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)b[0] * (DSP_IIR_T_ACC)w);
        for (k = 0; k < N; k++) {
            DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)b[k + 1] * (DSP_IIR_T_ACC)ws[k]);
        }
        for (k = N; k > 1; k--) { ws[k - 1] = ws[k - 2]; }
        if (N > 0) ws[0] = DSP_IIR_TPL_OUT(wacc, p);
        *y = DSP_IIR_TPL_OUT(yacc, p);
        return DSP_OK;
    }

    /* TDF2 مستقیم */
    {
        uint16_t N = p->order, k;
        const DSP_IIR_T_TYPE *b = p->pCoeffs;
        const DSP_IIR_T_TYPE *a = p->pCoeffs + N + 1;
        DSP_IIR_T_TYPE *s = p->pState;
        DSP_IIR_T_ACC yacc = 0;
        DSP_IIR_T_TYPE y0;
        DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)b[0] * (DSP_IIR_T_ACC)x);
        DSP_IIR_TPL_ACCUM(yacc, (DSP_IIR_T_ACC)s[0]);
        y0 = DSP_IIR_TPL_OUT(yacc, p);
        for (k = 0; k + 1 < N; k++) {
            DSP_IIR_T_ACC acc = 0;
            DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)b[k + 1] * (DSP_IIR_T_ACC)x);
            DSP_IIR_TPL_ACCUM(acc, -(DSP_IIR_T_ACC)a[k] * (DSP_IIR_T_ACC)y0);
            DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)s[k + 1]);
            s[k] = DSP_IIR_TPL_OUT(acc, p);
        }
        if (N > 0) {
            DSP_IIR_T_ACC acc = 0;
            DSP_IIR_TPL_ACCUM(acc, (DSP_IIR_T_ACC)b[N] * (DSP_IIR_T_ACC)x);
            DSP_IIR_TPL_ACCUM(acc, -(DSP_IIR_T_ACC)a[N - 1] * (DSP_IIR_T_ACC)y0);
            s[N - 1] = DSP_IIR_TPL_OUT(acc, p);
        }
        *y = y0;
        return DSP_OK;
    }
}

/* ---------------------------------------------------------------------------
 * پردازش بلاک
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_IIR_TPL_FN(process_block)(DSP_IIR_TPL_STRUCT *p, const DSP_IIR_T_TYPE *src,
                                        DSP_IIR_T_TYPE *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        DSP_IIR_T_TYPE y;
        DSP_IIR_TPL_FN(process_sample)(p, src[i], &y);
        dst[i] = y;
    }
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * به‌روزرسانی ضرایب
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_IIR_TPL_FN(update_coeffs)(DSP_IIR_TPL_STRUCT *p, const DSP_IIR_T_TYPE *coeffs)
{
    if (p == NULL || coeffs == NULL) return DSP_ERR_NULL_PTR;
    p->pCoeffs = coeffs;
    return DSP_OK;
}

#endif /* DSP_IIR_T_ENABLED */
#endif /* DSP_IIR_TPL_H */
