/**
 ******************************************************************************
 * @file    dsp_fir_tpl.h  (internal — template)
 * @brief   پیاده‌سازی مشترک FIR برای دقت‌های مختلف
 *
 * این فایل توسط فایل‌های زیر با تعریف ماکروها include می‌شود:
 *   dsp_fir_f32.c  dsp_fir_f64.c  dsp_fir_q15.c  dsp_fir_q31.c
 *
 * ماکروهای مورد نیاز (در هر فایل دقت‌محور):
 *   DSP_FIR_T_ENABLED  — ماکروی پیکربندی دقت (1/0)
 *   DSP_FIR_T_TYPE     — نوع نمونه
 *   DSP_FIR_T_ACC      — نوع accumulator
 *   DSP_FIR_T_SUFFIX   — f32 / f64 / q15 / q31
 *   DSP_FIR_T_FLOAT    — 1 ممیز شناور / 0 Fixed-Point
 *   DSP_FIR_T_CMSIS    — 1 اگر CMSIS این دقت را پشتیبانی می‌کند و فعال است
 *   DSP_FIR_TPL_OUT(acc, p)     — تبدیل accumulator به خروجی (با sat/shift)
 *   DSP_FIR_TPL_SATADD(a, b)    — جمع اشباعی (فقط Fixed)
 *   DSP_FIR_TPL_ARM_INIT(...)   — فراخوانی arm_fir_init_<sfx>
 *   DSP_FIR_TPL_ARM_PROC(...)   — فراخوانی arm_fir_<sfx>
 *
 * نکته‌ی معماری:
 *   - اگر CMSIS فعال باشد، پردازش به arm_fir_* واگذار می‌شود (قرارداد state
 *     یکسان است: بافر خطی به طول taps+blockSize-1).
 *   - حالت‌های TRANSPOSED/SYMMETRIC فقط در Backend داخلی وجود دارند؛ در حالت
 *     CMSIS در init خطای DSP_ERR_UNSUPPORTED برگردانده می‌شود.
 ******************************************************************************
 */

#ifndef DSP_FIR_TPL_H
#define DSP_FIR_TPL_H

#if DSP_FIR_T_ENABLED

#include "dsp_fir.h"
#include "dsp_port.h"

/* --- اتصال نام (دو مرحله: اول گسترش آرگومان، بعد paste) --- */
#define DSP_FIR_TPL_CAT2(a, b)  a##b
#define DSP_FIR_TPL_CAT(a, b)   DSP_FIR_TPL_CAT2(a, b)
#define DSP_FIR_TPL_STRUCT      DSP_FIR_TPL_CAT(DSP_FIR_TPL_CAT(dsp_fir_, DSP_FIR_T_SUFFIX), _t)
#define DSP_FIR_TPL_FN(name)    DSP_FIR_TPL_CAT(DSP_FIR_TPL_CAT(dsp_fir_, DSP_FIR_TPL_CAT(name, _)), DSP_FIR_T_SUFFIX)

/* ---------------------------------------------------------------------------
 * Init
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_FIR_TPL_FN(init)(DSP_FIR_TPL_STRUCT *p,
                               const DSP_FIR_T_TYPE *coeffs,
                               uint16_t taps, uint16_t blockSize, uint8_t mode,
                               DSP_FIR_T_TYPE *state, uint32_t stateLen,
                               uint8_t shift)
{
    uint32_t need;
    if (p == NULL || coeffs == NULL || state == NULL) return DSP_ERR_NULL_PTR;
    if (taps == 0 || blockSize == 0) return DSP_ERR_INVALID_PARAMETER;
    if (mode > DSP_FORM_SYMMETRIC) return DSP_ERR_INVALID_PARAMETER;
#if !DSP_FIR_T_FLOAT
    if (shift < 1 || shift > 31) return DSP_ERR_INVALID_PARAMETER;
#else
    DSP_UNUSED(shift);
#endif

    need = (mode == DSP_FORM_TRANSPOSED) ? (uint32_t)taps
                                         : DSP_FIR_STATE_SIZE(taps, blockSize);
    if (stateLen < need) return DSP_ERR_BUFFER_TOO_SMALL;

    p->taps      = taps;
    p->blockSize = blockSize;
    p->mode      = mode;
    p->pCoeffs   = coeffs;
    p->pState    = state;
    p->head      = 0;
    p->stateLen  = stateLen;
#if !DSP_FIR_T_FLOAT
    p->shift     = shift;
#endif

#if DSP_FIR_T_CMSIS
    if (mode != DSP_FORM_DIRECT) {
        return DSP_ERR_UNSUPPORTED;   /* CMSIS تنها Direct را دارد */
    }
    DSP_FIR_TPL_ARM_INIT(p, coeffs, taps, blockSize, state);
#else
    memset(state, 0, (size_t)need * sizeof(DSP_FIR_T_TYPE));
#endif
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * Reset
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_FIR_TPL_FN(reset)(DSP_FIR_TPL_STRUCT *p)
{
    uint32_t need;
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (p->pState == NULL) return DSP_ERR_NOT_INITIALIZED;

#if DSP_FIR_T_CMSIS
    DSP_FIR_TPL_ARM_INIT(p, p->pCoeffs, p->taps, p->blockSize, p->pState);
#else
    need = (p->mode == DSP_FORM_TRANSPOSED) ? (uint32_t)p->taps
                                            : DSP_FIR_STATE_SIZE(p->taps, p->blockSize);
    memset(p->pState, 0, (size_t)need * sizeof(DSP_FIR_T_TYPE));
    p->head = 0;
#endif
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * به‌روزرسانی ضرایب در زمان اجرا — تعویض اتمیک اشاره‌گر (بدون کپی)
 * بین دو بلاک انجام دهید؛ در حین process_block تغییر ندهید.
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_FIR_TPL_FN(update_coeffs)(DSP_FIR_TPL_STRUCT *p,
                                        const DSP_FIR_T_TYPE *coeffs, uint16_t n)
{
    if (p == NULL || coeffs == NULL) return DSP_ERR_NULL_PTR;
    if (n != p->taps) return DSP_ERR_INVALID_PARAMETER;

    p->pCoeffs = coeffs;
#if DSP_FIR_T_CMSIS
    p->arm.pCoeffs = coeffs;
#endif
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * پردازش نمونه‌ای
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_FIR_TPL_FN(process_sample)(DSP_FIR_TPL_STRUCT *p, DSP_FIR_T_TYPE x,
                                         DSP_FIR_T_TYPE *y)
{
    uint16_t taps, k;
    uint16_t head;

    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    taps = p->taps;

#if DSP_FIR_T_CMSIS
    DSP_FIR_TPL_ARM_PROC(p, &x, y, 1);
    return DSP_OK;
#else
    if (p->mode == DSP_FORM_TRANSPOSED) {
        DSP_FIR_T_ACC acc;
        acc = (DSP_FIR_T_ACC)p->pState[0] + (DSP_FIR_T_ACC)p->pCoeffs[0] * (DSP_FIR_T_ACC)x;
        for (k = 0; k + 1 < taps; k++) {
            p->pState[k] = (DSP_FIR_T_TYPE)((DSP_FIR_T_ACC)p->pState[k + 1] +
                          (DSP_FIR_T_ACC)p->pCoeffs[k + 1] * (DSP_FIR_T_ACC)x);
        }
        p->pState[taps - 1] = 0;
        *y = (DSP_FIR_T_TYPE)acc;
        return DSP_OK;
    }

    head = p->head;
    p->pState[head] = x;
    {
        DSP_FIR_T_ACC acc = 0;
        if (p->mode == DSP_FORM_SYMMETRIC) {
            uint16_t half = taps >> 1;
            for (k = 0; k < half; k++) {
                int32_t i1 = (int32_t)head - (int32_t)k;
                int32_t i2 = (int32_t)head - (int32_t)(taps - 1 - k);
                if (i1 < 0) i1 += taps;
                if (i2 < 0) i2 += taps;
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[k] *
                                 ((DSP_FIR_T_ACC)p->pState[i1] + (DSP_FIR_T_ACC)p->pState[i2]));
            }
            if (taps & 1u) {
                int32_t i1 = (int32_t)head - (int32_t)(taps >> 1);
                if (i1 < 0) i1 += taps;
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[taps >> 1] *
                                 (DSP_FIR_T_ACC)p->pState[i1]);
            }
        } else {
            for (k = 0; k < taps; k++) {
                int32_t idx = (int32_t)head - (int32_t)k;
                if (idx < 0) idx += taps;
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[k] *
                                 (DSP_FIR_T_ACC)p->pState[idx]);
            }
        }
        *y = DSP_FIR_TPL_OUT(acc, p);
    }
    head++;
    if (head >= taps) head = 0;
    p->head = head;
    return DSP_OK;
#endif
}

/* ---------------------------------------------------------------------------
 * پردازش بلاک
 * ------------------------------------------------------------------------- */
dsp_err_t DSP_FIR_TPL_FN(process_block)(DSP_FIR_TPL_STRUCT *p,
                                        const DSP_FIR_T_TYPE *src,
                                        DSP_FIR_T_TYPE *dst, uint16_t n)
{
    uint16_t taps, k, i;

    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    if (n == 0) return DSP_OK;
    if (n > p->blockSize) return DSP_ERR_INVALID_PARAMETER;
    taps = p->taps;

#if DSP_FIR_T_CMSIS
    DSP_FIR_TPL_ARM_PROC(p, src, dst, n);
    return DSP_OK;
#else
    if (p->pState == NULL) return DSP_ERR_NOT_INITIALIZED;

    if (p->mode == DSP_FORM_TRANSPOSED) {
        for (i = 0; i < n; i++) {
            DSP_FIR_T_ACC acc;
            acc = (DSP_FIR_T_ACC)p->pState[0] + (DSP_FIR_T_ACC)p->pCoeffs[0] * (DSP_FIR_T_ACC)src[i];
            for (k = 0; k + 1 < taps; k++) {
                p->pState[k] = (DSP_FIR_T_TYPE)((DSP_FIR_T_ACC)p->pState[k + 1] +
                              (DSP_FIR_T_ACC)p->pCoeffs[k + 1] * (DSP_FIR_T_ACC)src[i]);
            }
            p->pState[taps - 1] = 0;
            dst[i] = (DSP_FIR_T_TYPE)acc;
        }
        return DSP_OK;
    }

    /* Direct / Symmetric — append + rewind */
    if (p->stateLen < DSP_FIR_STATE_SIZE(taps, n)) return DSP_ERR_BUFFER_TOO_SMALL;

    for (i = 0; i < n; i++) {
        p->pState[taps - 1 + i] = src[i];
    }
    if (p->mode == DSP_FORM_SYMMETRIC) {
        uint16_t half = taps >> 1;
        for (i = 0; i < n; i++) {
            DSP_FIR_T_ACC acc = 0;
            for (k = 0; k < half; k++) {
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[k] *
                                 ((DSP_FIR_T_ACC)p->pState[taps - 1 + i - k] +
                                  (DSP_FIR_T_ACC)p->pState[i + k]));
            }
            if (taps & 1u) {
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[taps >> 1] *
                                 (DSP_FIR_T_ACC)p->pState[taps - 1 + i - (taps >> 1)]);
            }
            dst[i] = DSP_FIR_TPL_OUT(acc, p);
        }
    } else {
        for (i = 0; i < n; i++) {
            DSP_FIR_T_ACC acc = 0;
            for (k = 0; k < taps; k++) {
                DSP_FIR_TPL_ACC(acc, (DSP_FIR_T_ACC)p->pCoeffs[k] *
                                 (DSP_FIR_T_ACC)p->pState[taps - 1 + i - k]);
            }
            dst[i] = DSP_FIR_TPL_OUT(acc, p);
        }
    }
    /* rewind: آخرین taps-1 نمونه‌ی ورودی */
    for (k = 0; k + 1 < taps; k++) {
        p->pState[k] = p->pState[n + k];
    }
    return DSP_OK;
#endif
}

#endif /* DSP_FIR_T_ENABLED */
#endif /* DSP_FIR_TPL_H */
