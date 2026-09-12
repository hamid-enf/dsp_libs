/**
 * @file dsp_median.h — فیلتر میانه (Median) برای حذف Spike/Impulse
 *
 * ساختار: بافر حلقوی + آرایه‌ی مرتب کمکی با درج مرتب (O(W) در هر نمونه).
 * برای W ≤ 31 بهینه و بدون تخصیص. برای W بزرگ‌تر از روش Histogram
 * (پیشنهاد در مستندات) استفاده کنید.
 */
#ifndef DSP_MEDIAN_H
#define DSP_MEDIAN_H
#include "dsp_types.h"
#if DSP_ENABLE_MEDIAN
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t   window;    /* اندازه‌ی پنجره (فرد) */
    dsp_f32_t *pBuf;      /* پنجره نمونه (حلقه) */
    dsp_f32_t *pSorted;   /* نسخه‌ی مرتب (کاربر) */
    uint16_t   idx;
    uint16_t   count;
} dsp_median_f32_t;

dsp_err_t dsp_median_init_f32(dsp_median_f32_t *p, uint16_t window,
                              dsp_f32_t *buf, dsp_f32_t *sorted);
dsp_err_t dsp_median_process_f32(dsp_median_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_median_process_block_f32(dsp_median_f32_t *p, const dsp_f32_t *src,
                                       dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_median_reset_f32(dsp_median_f32_t *p);

/* نسخه‌ی Q15 — برای داده‌های حسگر ADC */
typedef struct {
    uint16_t   window;
    dsp_q15_t *pBuf;
    dsp_q15_t *pSorted;
    uint16_t   idx;
    uint16_t   count;
} dsp_median_q15_t;

dsp_err_t dsp_median_init_q15(dsp_median_q15_t *p, uint16_t window,
                              dsp_q15_t *buf, dsp_q15_t *sorted);
dsp_err_t dsp_median_process_q15(dsp_median_q15_t *p, dsp_q15_t x, dsp_q15_t *y);
dsp_err_t dsp_median_reset_q15(dsp_median_q15_t *p);

#if DSP_USE_SHORT_ALIASES
  #define Median_Init    dsp_median_init_f32
  #define Median_Process dsp_median_process_f32
#endif

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_MEDIAN */
#endif
