/**
 * @file dsp_adaptive.h — فیلترهای تطبیقی: LMS / NLMS / RLS (کاملاً Optional)
 *
 * LMS :  y[n] = wᵀ·x[n] ؛  e[n] = d[n] − y[n] ؛  w ← (1−μγ)·w + μ·e·x
 * NLMS:  μ_eff = μ / (‖x‖² + δ)
 * RLS :  k[n] = P·x/(λ + xᵀPx) ؛  w ← w + k·e ؛  P ← (P − k·xᵀP)/λ
 *
 * کاربرد: حذف نویز، حذف اکو، شناسایی سیستم، فیلتر تطبیقی حسگر.
 * RAM: LMS/NLMS = 2·len نمونه‌ها ؛ RLS = len·len برای ماتریس P.
 */
#ifndef DSP_ADAPTIVE_H
#define DSP_ADAPTIVE_H
#include "dsp_types.h"
#if DSP_ENABLE_LMS || DSP_ENABLE_NLMS || DSP_ENABLE_RLS
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t   len;         /* طول فیلتر */
    dsp_f32_t  mu;          /* گام یادگیری */
    dsp_f32_t  leakage;     /* 0 = بدون Leakage؛ >0 برای پایداری */
    dsp_f32_t *pW;          /* وزن‌ها (len) */
    dsp_f32_t *pX;          /* خط تأخیر (len) */
    uint16_t   idx;         /* ایندکس حلقه */
} dsp_lms_f32_t;

dsp_err_t dsp_lms_init_f32(dsp_lms_f32_t *p, uint16_t len, dsp_f32_t mu,
                           dsp_f32_t leakage, dsp_f32_t *weights, dsp_f32_t *delayline);
dsp_err_t dsp_lms_process_f32(dsp_lms_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                              dsp_f32_t *y, dsp_f32_t *e);
dsp_err_t dsp_lms_set_mu_f32(dsp_lms_f32_t *p, dsp_f32_t mu);
dsp_err_t dsp_lms_reset_f32(dsp_lms_f32_t *p);

typedef struct {
    uint16_t   len;
    dsp_f32_t  mu;
    dsp_f32_t  delta;       /* ε برای پایداری وقتی ‖x‖≈0 */
    dsp_f32_t *pW;
    dsp_f32_t *pX;
    uint16_t   idx;
} dsp_nlms_f32_t;

dsp_err_t dsp_nlms_init_f32(dsp_nlms_f32_t *p, uint16_t len, dsp_f32_t mu,
                            dsp_f32_t delta, dsp_f32_t *weights, dsp_f32_t *delayline);
dsp_err_t dsp_nlms_process_f32(dsp_nlms_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                               dsp_f32_t *y, dsp_f32_t *e);
dsp_err_t dsp_nlms_reset_f32(dsp_nlms_f32_t *p);

typedef struct {
    uint16_t   len;
    dsp_f32_t  lambda;      /* ضریب فراموشی (0.9..1.0) */
    dsp_f32_t  delta;       /* مقدار اولیه‌ی P (واریانس) */
    dsp_f32_t *pW;          /* len */
    dsp_f32_t *pX;          /* len */
    dsp_f32_t *pP;          /* len×len — ماتریس کوواریانس */
    uint16_t   idx;
} dsp_rls_f32_t;

dsp_err_t dsp_rls_init_f32(dsp_rls_f32_t *p, uint16_t len, dsp_f32_t lambda,
                           dsp_f32_t delta, dsp_f32_t *weights, dsp_f32_t *delayline,
                           dsp_f32_t *cov);
dsp_err_t dsp_rls_process_f32(dsp_rls_f32_t *p, dsp_f32_t x, dsp_f32_t d,
                              dsp_f32_t *y, dsp_f32_t *e);
dsp_err_t dsp_rls_reset_f32(dsp_rls_f32_t *p);

#if DSP_USE_SHORT_ALIASES
  #define LMS_Init    dsp_lms_init_f32
  #define LMS_Process dsp_lms_process_f32
  #define NLMS_Init   dsp_nlms_init_f32
  #define NLMS_Process dsp_nlms_process_f32
  #define RLS_Init    dsp_rls_init_f32
  #define RLS_Process dsp_rls_process_f32
#endif

#ifdef __cplusplus
}
#endif
#endif /* any adaptive */
#endif
