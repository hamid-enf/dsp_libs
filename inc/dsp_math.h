/**
 ******************************************************************************
 * @file    dsp_math.h
 * @brief   موتور ریاضی داخلی Design-Time (فقط در زمان طراحی ضرایب استفاده
 *          می‌شود؛ در مسیر Real-Time پردازش هیچ وابستگی به این فایل نیست)
 *
 * شامل:
 *   - عدد مختلط float64 و عملیات پایه
 *   - انتگرال بیضوی کامل نوع اول  K(m)   (روش Arithmetic-Geometric Mean)
 *   - توابع بیضوی ژاکوبی sn/cn/dn          (سری تتا + nome)
 *   - معکوس sn (Landen) و sc — برای طراحی فیلتر بیضوی (Cauer)
 *   - حل معادله‌ی درجه‌ی بیضوی (nomes) — برای ellipdeg
 *   - ریشه‌یابی چندجمله‌ای (Durand-Kerner) — برای فیلتر Bessel
 ******************************************************************************
 */

#ifndef DSP_MATH_H
#define DSP_MATH_H

#include "dsp_port.h"
#include "dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * عدد مختلط float64
 * ------------------------------------------------------------------------- */
typedef struct {
    double re;
    double im;
} dsp_cx_t;

static inline dsp_cx_t dsp_cx_make(double re, double im) { dsp_cx_t c; c.re = re; c.im = im; return c; }
static inline dsp_cx_t dsp_cx_add(dsp_cx_t a, dsp_cx_t b) { return dsp_cx_make(a.re + b.re, a.im + b.im); }
static inline dsp_cx_t dsp_cx_sub(dsp_cx_t a, dsp_cx_t b) { return dsp_cx_make(a.re - b.re, a.im - b.im); }
static inline dsp_cx_t dsp_cx_mul(dsp_cx_t a, dsp_cx_t b) {
    return dsp_cx_make(a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re);
}
static inline dsp_cx_t dsp_cx_scale(dsp_cx_t a, double s) { return dsp_cx_make(a.re * s, a.im * s); }
static inline double  dsp_cx_abs(dsp_cx_t a) { return hypot(a.re, a.im); }
static inline dsp_cx_t dsp_cx_conj(dsp_cx_t a) { return dsp_cx_make(a.re, -a.im); }

dsp_cx_t dsp_cx_div(dsp_cx_t a, dsp_cx_t b);
dsp_cx_t dsp_cx_sqrt(dsp_cx_t a);   /* ریشه‌ی اصلی (principal) */
dsp_cx_t dsp_cx_exp(dsp_cx_t a);

/* ---------------------------------------------------------------------------
 * توابع بیضوی — همه‌ی آرگومان‌ها و خروجی‌ها double
 * ------------------------------------------------------------------------- */
double dsp_ellipk(double m);          /* K(m) — انتگرال بیضوی کامل نوع اول */
double dsp_ellipkm1(double m);        /* K(1-m) */
void   dsp_ellipj(double u, double m, double *sn, double *cn, double *dn);
dsp_cx_t dsp_arc_jac_sn(dsp_cx_t w, double m);   /* z به‌طوری که sn(z,m)=w */
double dsp_arc_jac_sc1(double w, double m);      /* معکوس sc با مدول مکمل */
double dsp_ellipdeg(int n, double m1);           /* حل معادله‌ی درجه‌ی بیضوی */

/* ---------------------------------------------------------------------------
 * ریشه‌یابی چندجمله‌ای — روش Durand-Kerner
 * p: ضرایب چندجمله‌ای (بالاترین توان اول)، n: درجه
 * roots: خروجی، آرایه‌ای به طول n
 * ------------------------------------------------------------------------- */
dsp_err_t dsp_poly_roots(const double *p, int n, dsp_cx_t *roots);

/* ضرایب چندجمله‌ای بسل y_N(s) = sum_{k=0..N} (N+k)!/((N-k)! k! 2^k) s^k */
void dsp_bessel_poly_coeffs(int order, double *coeffs /* length order+1, coeffs[i] for s^i */);

#ifdef __cplusplus
}
#endif

#endif /* DSP_MATH_H */
