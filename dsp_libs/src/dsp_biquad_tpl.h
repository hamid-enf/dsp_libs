/**
 ******************************************************************************
 * @file    dsp_biquad_tpl.h  (internal — template)
 * @brief   پیاده‌سازی مشترک Biquad (تک‌طبقه + Cascade/SOS) برای دقت‌های مختلف
 *
 * فایل‌های includeکننده: dsp_biquad_f32.c  dsp_biquad_f64.c  dsp_biquad_q15.c  dsp_biquad_q31.c
 *
 * ماکروهای مورد نیاز:
 *   DSP_BQ_T_ENABLED   — ماکروی پیکربندی دقت
 *   DSP_BQ_T_TYPE      — نوع نمونه
 *   DSP_BQ_T_ACC       — نوع accumulator
 *   DSP_BQ_T_SUFFIX    — f32/f64/q15/q31
 *   DSP_BQ_T_FLOAT     — 1 شناور / 0 Fixed
 *   DSP_BQ_TPL_OUT(acc, p)          — تبدیل accumulator به خروجی
 *   DSP_BQ_TPL_SATADD(a, b)         — جمع اشباعی
 *   DSP_BQ_TPL_ACC(acc, expr)       — جمع با/بدون اشباع
 *   DSP_BQ_TPL_PARAM_STORE(p, c)    — ذخیره‌ی ضرایب RBJ در ساختار (شامل کمی‌سازی)
 *
 * معادلات:
 *   DF1 : y = b0x + b1x1 + b2x2 - a1y1 - a2y2
 *   DF2 : w = x - a1w1 - a2w2 ;  y = b0w + b1w1 + b2w2
 *   TDF2: y = b0x + d1 ;  d1 = b1x - a1y + d2 ;  d2 = b2x - a2y
 ******************************************************************************
 */

#ifndef DSP_BIQUAD_TPL_H
#define DSP_BIQUAD_TPL_H

#if DSP_BQ_T_ENABLED

#include "dsp_biquad.h"
#include "dsp_biquad_int.h"
#include "dsp_port.h"

#define DSP_BQ_TPL_CAT2(a, b)  a##b
#define DSP_BQ_TPL_CAT(a, b)   DSP_BQ_TPL_CAT2(a, b)
#define DSP_BQ_TPL_STRUCT      DSP_BQ_TPL_CAT(DSP_BQ_TPL_CAT(dsp_biquad_, DSP_BQ_T_SUFFIX), _t)
#define DSP_BQ_TPL_CASCADE     DSP_BQ_TPL_CAT(DSP_BQ_TPL_CAT(dsp_biquad_cascade_, DSP_BQ_T_SUFFIX), _t)
#define DSP_BQ_TPL_FN(name)    DSP_BQ_TPL_CAT(DSP_BQ_TPL_CAT(dsp_biquad_, DSP_BQ_TPL_CAT(name, _)), DSP_BQ_T_SUFFIX)
#define DSP_BQ_TPL_CFN(name)   DSP_BQ_TPL_CAT(DSP_BQ_TPL_CAT(dsp_biquad_cascade_, DSP_BQ_TPL_CAT(name, _)), DSP_BQ_T_SUFFIX)

/* --- پیش‌اعلام توابعی که به ترتیب بعدی تعریف می‌شوند --- */
dsp_err_t DSP_BQ_TPL_FN(reset)(DSP_BQ_TPL_STRUCT *p);

/* ---------------------------------------------------------------------------
 * پردازش یک نمونه — بر اساس فرم
 * ------------------------------------------------------------------------- */
#define DSP_BQ_TPL_STEP(p, x, out)                                            \
    do {                                                                      \
        if ((p)->form == DSP_DF1) {                                           \
            DSP_BQ_T_ACC acc = 0;                                             \
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)(p)->b0 * (DSP_BQ_T_ACC)(x));   \
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)(p)->b1 * (DSP_BQ_T_ACC)(p)->x1);\
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)(p)->b2 * (DSP_BQ_T_ACC)(p)->x2);\
            DSP_BQ_TPL_ACC(acc, -(DSP_BQ_T_ACC)(p)->a1 * (DSP_BQ_T_ACC)(p)->y1);\
            DSP_BQ_TPL_ACC(acc, -(DSP_BQ_T_ACC)(p)->a2 * (DSP_BQ_T_ACC)(p)->y2);\
            (p)->x2 = (p)->x1; (p)->x1 = (x);                                 \
            (out) = DSP_BQ_TPL_OUT(acc, p);                                   \
            (p)->y2 = (p)->y1; (p)->y1 = (out);                               \
        } else if ((p)->form == DSP_DF2) {                                    \
            DSP_BQ_T_ACC wacc = 0, yacc = 0;                                   \
            DSP_BQ_TPL_ACC(wacc, (DSP_BQ_T_ACC)(x));                          \
            DSP_BQ_TPL_ACC(wacc, -(DSP_BQ_T_ACC)(p)->a1 * (DSP_BQ_T_ACC)(p)->d1);\
            DSP_BQ_TPL_ACC(wacc, -(DSP_BQ_T_ACC)(p)->a2 * (DSP_BQ_T_ACC)(p)->d2);\
            DSP_BQ_TPL_ACC(yacc, (DSP_BQ_T_ACC)(p)->b0 * wacc);               \
            DSP_BQ_TPL_ACC(yacc, (DSP_BQ_T_ACC)(p)->b1 * (DSP_BQ_T_ACC)(p)->d1);\
            DSP_BQ_TPL_ACC(yacc, (DSP_BQ_T_ACC)(p)->b2 * (DSP_BQ_T_ACC)(p)->d2);\
            (p)->d2 = (p)->d1;                                                \
            (p)->d1 = DSP_BQ_TPL_OUT(wacc, p);                                \
            (out) = DSP_BQ_TPL_OUT(yacc, p);                                  \
        } else {  /* DSP_TDF2 */                                              \
            DSP_BQ_T_ACC yacc = 0, nd1 = 0, nd2 = 0;                          \
            DSP_BQ_TPL_ACC(yacc, (DSP_BQ_T_ACC)(p)->b0 * (DSP_BQ_T_ACC)(x));  \
            DSP_BQ_TPL_ACC(yacc, (DSP_BQ_T_ACC)(p)->d1);                      \
            (out) = DSP_BQ_TPL_OUT(yacc, p);                                  \
            DSP_BQ_TPL_ACC(nd1, (DSP_BQ_T_ACC)(p)->b1 * (DSP_BQ_T_ACC)(x));   \
            DSP_BQ_TPL_ACC(nd1, -(DSP_BQ_T_ACC)(p)->a1 * (DSP_BQ_T_ACC)(out));\
            DSP_BQ_TPL_ACC(nd1, (DSP_BQ_T_ACC)(p)->d2);                       \
            (p)->d1 = DSP_BQ_TPL_OUT(nd1, p);                                 \
            DSP_BQ_TPL_ACC(nd2, (DSP_BQ_T_ACC)(p)->b2 * (DSP_BQ_T_ACC)(x));   \
            DSP_BQ_TPL_ACC(nd2, -(DSP_BQ_T_ACC)(p)->a2 * (DSP_BQ_T_ACC)(out));\
            (p)->d2 = DSP_BQ_TPL_OUT(nd2, p);                                 \
        }                                                                     \
    } while (0)

/* ---------------------------------------------------------------------------
 * Init — فقط فرم را تنظیم و حالت را صفر می‌کند
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(init)(DSP_BQ_TPL_STRUCT *p, uint8_t form)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (form != DSP_DF1 && form != DSP_DF2 && form != DSP_TDF2) {
        return DSP_ERR_INVALID_PARAMETER;
    }
#if !DSP_BQ_T_FLOAT
    /* در Fixed-Point، فقط DF1 پشتیبانی می‌شود: حالت‌های DF2/TDF2 نیازمند
       ذخیره‌ی accumulator-scale هستند که در قالب نمونه‌ی 16/32 بیتی نمی‌گنجد.
       (همان رویه‌ی CMSIS: arm_biquad_cascade_df1_q15/q31) */
    if (form != DSP_DF1) return DSP_ERR_UNSUPPORTED;
#endif
    memset(p, 0, sizeof(*p));
    p->form = form;
    p->type = DSP_BIQUAD_RAW;
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * تنظیم پارامترها و محاسبه‌ی ضرایب (RBJ)
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(set_params)(DSP_BQ_TPL_STRUCT *p, uint8_t type,
                                    uint32_t fs, DSP_BQ_T_FC fc,
                                    DSP_BQ_T_FC q, DSP_BQ_T_FC gain_db)
{
    double c[5];
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (type > DSP_BIQUAD_HSHELF) return DSP_ERR_INVALID_PARAMETER;
    if (fs == 0) return DSP_ERR_INVALID_PARAMETER;
    if (fc <= 0.0 || fc >= (DSP_BQ_T_FC)(fs >> 1)) return DSP_ERR_INVALID_PARAMETER;
    if (q <= 0.0) return DSP_ERR_INVALID_PARAMETER;

    dsp_biquad_rbj_f64(type, fs, (double)fc, (double)q, (double)gain_db, c);

    DSP_BQ_TPL_PARAM_STORE(p, c);

    p->type    = type;
    p->fs      = fs;
    p->fc      = fc;
    p->q       = q;
    p->gain_db = gain_db;
    DSP_BQ_TPL_FN(reset)(p);   /* تغییر ضرایب => پاک‌سازی حالت */
    return DSP_OK;
}

#if DSP_BQ_T_FLOAT
/* ---------------------------------------------------------------------------
 * دریافت ضرایب (فقط شناور؛ Fixed در فایل دقت‌محور با خروجی shift)
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(get_coeffs)(const DSP_BQ_TPL_STRUCT *p, DSP_BQ_T_TYPE c[5])
{
    if (p == NULL || c == NULL) return DSP_ERR_NULL_PTR;
    c[0] = p->b0; c[1] = p->b1; c[2] = p->b2; c[3] = p->a1; c[4] = p->a2;
    return DSP_OK;
}
#endif

/* ---------------------------------------------------------------------------
 * Reset
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(reset)(DSP_BQ_TPL_STRUCT *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->x1 = p->x2 = p->y1 = p->y2 = (DSP_BQ_T_TYPE)0;
    p->d1 = p->d2 = (DSP_BQ_T_TYPE)0;
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * پردازش نمونه‌ای
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(process_sample)(DSP_BQ_TPL_STRUCT *p, DSP_BQ_T_TYPE x,
                                        DSP_BQ_T_TYPE *y)
{
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    DSP_BQ_TPL_STEP(p, x, *y);
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * پردازش بلاک
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_BQ_TPL_FN(process_block)(DSP_BQ_TPL_STRUCT *p, const DSP_BQ_T_TYPE *src,
                                       DSP_BQ_T_TYPE *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        DSP_BQ_T_TYPE y;
        DSP_BQ_TPL_STEP(p, src[i], y);
        dst[i] = y;
    }
    return DSP_OK;
}

/* ===========================================================================
 * Cascade (SOS)
 * =========================================================================== */
dsp_err_t DSP_BQ_TPL_CFN(init)(DSP_BQ_TPL_CASCADE *p, uint16_t stages,
                               const DSP_BQ_T_TYPE *coeffs
#if !DSP_BQ_T_FLOAT
                               , uint8_t shift
#endif
                               , DSP_BQ_T_TYPE *state,
                               uint32_t stateLen, uint8_t form)
{
    uint32_t perStage;
    if (p == NULL || coeffs == NULL || state == NULL) return DSP_ERR_NULL_PTR;
    if (stages == 0) return DSP_ERR_INVALID_PARAMETER;
    if (form != DSP_DF1 && form != DSP_TDF2) return DSP_ERR_INVALID_PARAMETER;
#if !DSP_BQ_T_FLOAT
    if (form != DSP_DF1) return DSP_ERR_UNSUPPORTED;
#endif

    perStage = (form == DSP_DF1) ? 4u : 2u;
    if (stateLen < (uint32_t)stages * perStage) return DSP_ERR_BUFFER_TOO_SMALL;

    p->stages  = stages;
    p->form    = form;
    p->pCoeffs = coeffs;
    p->pState  = state;
    p->stateLen = stateLen;
#if !DSP_BQ_T_FLOAT
    p->shift   = shift;
#endif
    memset(state, 0, (size_t)stages * perStage * sizeof(DSP_BQ_T_TYPE));
    return DSP_OK;
}

dsp_err_t DSP_BQ_TPL_CFN(reset)(DSP_BQ_TPL_CASCADE *p)
{
    uint32_t perStage;
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (p->pState == NULL) return DSP_ERR_NOT_INITIALIZED;
    perStage = (p->form == DSP_DF1) ? 4u : 2u;
    memset(p->pState, 0, (size_t)p->stages * perStage * sizeof(DSP_BQ_T_TYPE));
    return DSP_OK;
}

dsp_err_t DSP_BQ_TPL_CFN(update_coeffs)(DSP_BQ_TPL_CASCADE *p,
                                        const DSP_BQ_T_TYPE *coeffs, uint16_t stages)
{
    if (p == NULL || coeffs == NULL) return DSP_ERR_NULL_PTR;
    if (stages != p->stages) return DSP_ERR_INVALID_PARAMETER;
    p->pCoeffs = coeffs;
    return DSP_OK;
}

dsp_err_t DSP_BQ_TPL_CFN(process_sample)(DSP_BQ_TPL_CASCADE *p, DSP_BQ_T_TYPE x,
                                         DSP_BQ_T_TYPE *y)
{
    uint16_t s;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;

    if (p->form == DSP_DF1) {
        const DSP_BQ_T_TYPE *c = p->pCoeffs;
        DSP_BQ_T_TYPE *st = p->pState;   /* 4 در هر طبقه: x1 x2 y1 y2 */
        for (s = 0; s < p->stages; s++) {
            DSP_BQ_T_ACC acc = 0;
            DSP_BQ_T_TYPE y0;
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)c[0] * (DSP_BQ_T_ACC)x);
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)c[1] * (DSP_BQ_T_ACC)st[0]);
            DSP_BQ_TPL_ACC(acc, (DSP_BQ_T_ACC)c[2] * (DSP_BQ_T_ACC)st[1]);
            DSP_BQ_TPL_ACC(acc, -(DSP_BQ_T_ACC)c[3] * (DSP_BQ_T_ACC)st[2]);
            DSP_BQ_TPL_ACC(acc, -(DSP_BQ_T_ACC)c[4] * (DSP_BQ_T_ACC)st[3]);
            y0 = DSP_BQ_TPL_OUT(acc, p);
            st[1] = st[0]; st[0] = x;
            st[3] = st[2]; st[2] = y0;
            x = y0;
            c += 5; st += 4;
        }
        *y = x;
        return DSP_OK;
    }

    /* TDF2 — 2 حالت در هر طبقه */
    {
        const DSP_BQ_T_TYPE *c = p->pCoeffs;
        DSP_BQ_T_TYPE *st = p->pState;
        for (s = 0; s < p->stages; s++) {
            DSP_BQ_T_ACC y = 0, d1 = 0, d2 = 0;
            DSP_BQ_T_TYPE y0;
            DSP_BQ_TPL_ACC(y, (DSP_BQ_T_ACC)c[0] * (DSP_BQ_T_ACC)x);
            DSP_BQ_TPL_ACC(y, (DSP_BQ_T_ACC)st[0]);
            y0 = DSP_BQ_TPL_OUT(y, p);
            DSP_BQ_TPL_ACC(d1, (DSP_BQ_T_ACC)c[1] * (DSP_BQ_T_ACC)x);
            DSP_BQ_TPL_ACC(d1, -(DSP_BQ_T_ACC)c[3] * (DSP_BQ_T_ACC)y0);
            DSP_BQ_TPL_ACC(d1, (DSP_BQ_T_ACC)st[1]);
            st[0] = DSP_BQ_TPL_OUT(d1, p);
            DSP_BQ_TPL_ACC(d2, (DSP_BQ_T_ACC)c[2] * (DSP_BQ_T_ACC)x);
            DSP_BQ_TPL_ACC(d2, -(DSP_BQ_T_ACC)c[4] * (DSP_BQ_T_ACC)y0);
            st[1] = DSP_BQ_TPL_OUT(d2, p);
            x = y0;
            c += 5; st += 2;
        }
        *y = x;
    }
    return DSP_OK;
}

dsp_err_t DSP_BQ_TPL_CFN(process_block)(DSP_BQ_TPL_CASCADE *p, const DSP_BQ_T_TYPE *src,
                                        DSP_BQ_T_TYPE *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        DSP_BQ_T_TYPE y;
        DSP_BQ_TPL_CFN(process_sample)(p, src[i], &y);
        dst[i] = y;
    }
    return DSP_OK;
}

#endif /* DSP_BQ_T_ENABLED */
#endif /* DSP_BIQUAD_TPL_H */
