/**
 * @file dsp_savgol.h — فیلتر Savitzky-Golay (هموارسازی + تخمین مشتق)
 *
 * y[n] = Σ_{k=-M}^{M} g_k · x[n-k]    (برای Smoothing، مشتق‌مرتبه‌ی d)
 *
 * ضرایب g_k با برازش چندجمله‌ای مرتبه‌ی P روی پنجره‌ی 2M+1 نقطه و مشتق‌گیری
 * از مرتبه‌ی d به دست می‌آیند (روش کمترین مربعات — در dsp_savgol.c).
 * خروجی به اندازه‌ی M نمونه تأخیر دارد (پنجره‌ی مرکزی).
 */
#ifndef DSP_SAVGOL_H
#define DSP_SAVGOL_H
#include "dsp_types.h"
#if DSP_ENABLE_SAVGOL
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t        m;        /* نیم‌پنجره: پنجره = 2m+1 */
    const dsp_f32_t *pCoeffs; /* 2m+1 ضریب */
    dsp_f32_t       *pBuf;    /* 2m+1 نمونه (حلقه) */
    uint16_t        idx;
} dsp_savgol_f32_t;

dsp_err_t dsp_savgol_init_f32(dsp_savgol_f32_t *p, uint16_t m,
                              const dsp_f32_t *coeffs, dsp_f32_t *buf);
dsp_err_t dsp_savgol_process_f32(dsp_savgol_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_savgol_reset_f32(dsp_savgol_f32_t *p);

/* طراحی ضرایب:
   window = 2m+1 (≤ DSP_MAX_SG_WINDOW)، polyOrder ≤ window-1، deriv درجه‌ی مشتق
   خروجی: 2m+1 ضریب در coeffsOut */
dsp_err_t dsp_savgol_design_f32(uint16_t m, uint16_t polyOrder, uint16_t deriv,
                                dsp_f32_t *coeffsOut);

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_SAVGOL */
#endif
