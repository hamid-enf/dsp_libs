/**
 * @file dsp_kalman.h — فیلتر کالمن 1D (قابل توسعه به چندبعدی/EKF)
 *
 * مدل:  x_k = x_{k-1} + w_k   (w ~ N(0,Q))   — دینامیک ثابت
 *       z_k = x_k + v_k       (v ~ N(0,R))
 *
 * پیش‌بینی:  x̂⁻ = x̂ ؛  P⁻ = P + Q
 * به‌روزرسانی: K = P⁻/(P⁻+R) ؛  x̂ = x̂⁻ + K(z − x̂⁻) ؛  P = (1−K)P⁻
 *
 * RAM: 5 عدد float — سبک‌ترین فیلتر کتابخانه.
 */
#ifndef DSP_KALMAN_H
#define DSP_KALMAN_H
#include "dsp_types.h"
#if DSP_ENABLE_KALMAN
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_f32_t q;        /* واریانس نویز فرآیند */
    dsp_f32_t r;        /* واریانس نویز اندازه‌گیری */
    dsp_f32_t x;        /* تخمین حالت */
    dsp_f32_t p;        /* کوواریانس خطای تخمین */
    dsp_f32_t k;        /* گین کالمن (آخرین) */
} dsp_kalman1d_f32_t;

dsp_err_t dsp_kalman1d_init_f32(dsp_kalman1d_f32_t *p, dsp_f32_t x0,
                                dsp_f32_t p0, dsp_f32_t q, dsp_f32_t r);
dsp_err_t dsp_kalman1d_set_noise_f32(dsp_kalman1d_f32_t *p, dsp_f32_t q, dsp_f32_t r);
dsp_err_t dsp_kalman1d_predict_f32(dsp_kalman1d_f32_t *p);      /* فقط پیش‌بینی */
dsp_err_t dsp_kalman1d_update_f32(dsp_kalman1d_f32_t *p, dsp_f32_t z, dsp_f32_t *xout);
dsp_err_t dsp_kalman1d_process_f32(dsp_kalman1d_f32_t *p, dsp_f32_t z, dsp_f32_t *xout);
dsp_err_t dsp_kalman1d_reset_f32(dsp_kalman1d_f32_t *p, dsp_f32_t x0, dsp_f32_t p0);

#if DSP_USE_SHORT_ALIASES
  #define Kalman_Init   dsp_kalman1d_init_f32
  #define Kalman_Update dsp_kalman1d_process_f32
#endif

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_KALMAN */
#endif
