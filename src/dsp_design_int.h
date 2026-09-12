/**
 ******************************************************************************
 * @file    dsp_design_int.h  (internal)
 * @brief   هسته‌ی مشترک موتور طراحی — تبدیل‌های ZPK و Bilinear و ساخت SOS
 ******************************************************************************
 */

#ifndef DSP_DESIGN_INT_H
#define DSP_DESIGN_INT_H

#include "dsp_math.h"

#define DSP_ZPK_MAX 96

typedef struct {
    int      nz;          /* تعداد صفرها */
    int      np;          /* تعداد قطب‌ها */
    dsp_cx_t z[DSP_ZPK_MAX];
    dsp_cx_t p[DSP_ZPK_MAX];
    double   k;
} dsp_zpk_t;

/* حاصل‌ضرب (−v_i) — برای گین پروتوتایپ */
double dsp_prod_neg(const dsp_cx_t *v, int n);

/* تبدیل‌های فرکانسی آنالوگ (فرکانس‌ها پیش‌وارپ‌شده‌اند: Ω = tan(π f / fs)) */
void dsp_design_lp2lp(dsp_zpk_t *zpk, double wo);
void dsp_design_lp2hp(dsp_zpk_t *zpk, double wo);
void dsp_design_lp2bp(dsp_zpk_t *zpk, double wo, double bw);
void dsp_design_lp2bs(dsp_zpk_t *zpk, double wo, double bw);

/* Bilinear با T=2:  s = (z-1)/(z+1)   =>   z_d = (1+s)/(1-s)
   (همان قرارداد scipy: butter(1, 0.5) قطبِ صفر می‌دهد) */
void dsp_design_bilinear(dsp_zpk_t *zpk);

/* ساخت SOS نرمال‌شده از ZPK دیجیتال.
   cfg: پارامترهای طراحی (برای تعیین فرکانس مرجع نرمال‌سازی)
   خروجی در cfg->pSOS و cfg->pNumSections و cfg->pGain نوشته می‌شود. */
dsp_err_t dsp_design_zpk2sos(const dsp_design_params_t *cfg, const dsp_zpk_t *zpk,
                               double target_gain);

/* ارزیابی H(z) برای یک SOS در z = e^{jw} — مختلط */
dsp_cx_t dsp_design_sos_eval(const dsp_f64_t *sos, uint16_t nSections, dsp_f64_t w);

#endif /* DSP_DESIGN_INT_H */

/* اجرای کامل خط لوله: prototype (zpk) → تبدیل فرکانسی (بر اساس cfg->type با
   پیش‌وارپ) → Bilinear → SOS نرمال‌شده.
   target_gain: گین هدف در فرکانس مرجع (برای Chebyshev I/Elliptic زوج = 10^(-rp/20)). */
dsp_err_t dsp_design_run(const dsp_design_params_t *cfg,
                         const dsp_cx_t *poles, int np,
                         const dsp_cx_t *zeros, int nz, double k,
                         double target_gain);
