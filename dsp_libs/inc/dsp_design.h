/**
 ******************************************************************************
 * @file    dsp_design.h
 * @brief   موتور طراحی خودکار ضرایب فیلتر (Design-Time)
 *
 * پشتیبانی از: Butterworth / Chebyshev I / Chebyshev II / Elliptic (Cauer) / Bessel
 * انواع: LP / HP / BP / BS
 *
 * خروجی همیشه به صورت Cascade of Second-Order Sections (SOS) نرمال‌شده است:
 *   pSOS = [b0 b1 b2 a1 a2] × nSections  (با a0=1)
 * و به‌گونه‌ای نرمال‌شده که گین کل در فرکانس مرجع نوع فیلتر برابر 1 باشد
 * (LP/BS در DC، HP در Nyquist، BP/BS در مرکز باند).
 *
 * دقت محاسبات: float64 در داخل موتور. سپس با توابع تبدیل می‌توانید ضرایب را
 * به float32 یا Fixed-Point (Q15/Q31) تبدیل کنید.
 *
 * نکته: طراحی در مسیر Real-Time نیست؛ فقط در Init (یک بار) اجرا می‌شود.
 * برای فیلترهای Elliptic در M7 با FPU نرم‌افزاری، طراحی ممکن است چند
 * میلی‌ثانیه طول بکشد — بهتر است به‌صورت Offline انجام و ضرایب در Flash
 * ذخیره شود.
 ******************************************************************************
 */

#ifndef DSP_DESIGN_H
#define DSP_DESIGN_H

#include "dsp_types.h"

#if DSP_ENABLE_BUTTERWORTH || DSP_ENABLE_CHEBYSHEV || DSP_ENABLE_ELLIPTIC || DSP_ENABLE_BESSEL

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * پارامترهای طراحی
 * ------------------------------------------------------------------------- */
typedef struct {
    uint16_t     type;       /* DSP_FILTER_LP/HP/BP/BS */
    uint16_t     order;      /* مرتبه‌ی پروتوتایپ (برای BP/BS مرتبه‌ی نهایی 2×order است) */
    dsp_f64_t    fs;         /* فرکانس نمونه‌برداری (Hz) */
    dsp_f64_t    fc;         /* LP/HP: فرکانس قطع؛ BP/BS: لبه‌ی پایین باند */
    dsp_f64_t    fc2;        /* BP/BS: لبه‌ی بالای باند (برای LP/HP بی‌استفاده) */
    dsp_f64_t    ripple_db;  /* Chebyshev I / Elliptic: موج‌دار بودن پاس‌باند (dB) */
    dsp_f64_t    stop_db;    /* Chebyshev II / Elliptic: تضعیف استاپ‌باند (dB) */
    /* خروجی */
    dsp_f64_t    *pSOS;      /* بافر خروجی: 5 × nSections (حداکثر DSP_MAX_SOS_STAGES) */
    uint16_t     *pNumSections; /* خروجی: تعداد طبقات */
    dsp_f64_t    *pGain;     /* خروجی: گین کلی (اختیاری، NULL مجاز) */
} dsp_design_params_t;

/* ---------------------------------------------------------------------------
 * طراح‌ها — همه DSP_OK برمی‌گردانند یا کد خطای مربوطه
 * ------------------------------------------------------------------------- */
#if DSP_ENABLE_BUTTERWORTH
dsp_err_t dsp_design_butterworth(const dsp_design_params_t *cfg);
#endif

#if DSP_ENABLE_CHEBYSHEV
dsp_err_t dsp_design_chebyshev1(const dsp_design_params_t *cfg);
dsp_err_t dsp_design_chebyshev2(const dsp_design_params_t *cfg);
#endif

#if DSP_ENABLE_ELLIPTIC
dsp_err_t dsp_design_elliptic(const dsp_design_params_t *cfg);
#endif

#if DSP_ENABLE_BESSEL
dsp_err_t dsp_design_bessel(const dsp_design_params_t *cfg);
#endif

/* ---------------------------------------------------------------------------
 * توابع کمکی عمومی طراحی
 * ------------------------------------------------------------------------- */
/* تخمین مرتبه‌ی لازم برای Butterworth LP با مشخصات داده‌شده (برای انتخاب order) */
dsp_f64_t dsp_design_butterworth_estimate_order(dsp_f64_t fs, dsp_f64_t fpass,
                                                dsp_f64_t fstop, dsp_f64_t apass_db,
                                                dsp_f64_t astop_db);

/* تبدیل ضرایب SOS از f64 به f32 (دقت‌محور) */
void dsp_design_sos_f64_to_f32(const dsp_f64_t *src, uint16_t nSections,
                               dsp_f32_t *dst);

/* تبدیل ضرایب SOS به Fixed-Point:
   یک shift مشترک برای همه‌ی طبقات انتخاب می‌شود (دقیق‌ترین حالت).
   خروجی shift را در *pShift برمی‌گرداند. */
dsp_err_t dsp_design_sos_f64_to_q15(const dsp_f64_t *src, uint16_t nSections,
                                    dsp_q15_t *dst, uint8_t *pShift);
dsp_err_t dsp_design_sos_f64_to_q31(const dsp_f64_t *src, uint16_t nSections,
                                    dsp_q31_t *dst, uint8_t *pShift);

/* ارزیابی پاسخ اندازه‌ی یک فیلتر SOS در فرکانس نرمال‌شده w (رادیان/نمونه):
   برای تست و Validation مفید است. */
dsp_f64_t dsp_design_sos_magnitude(const dsp_f64_t *sos, uint16_t nSections,
                                   dsp_f64_t w);

#ifdef __cplusplus
}
#endif

#endif /* any design feature */
#endif /* DSP_DESIGN_H */
