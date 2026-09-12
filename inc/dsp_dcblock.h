/**
 * @file dsp_dcblock.h — حذف مؤلفه‌ی DC (DC Blocker)
 *
 * y[n] = x[n] − x[n−1] + R·y[n−1]      (R معمولاً 0.995 تا 0.999)
 * بدون حذف فرکانس‌های بالاتر؛ مناسب: ADC → DC Removal → Filter → FFT
 */
#ifndef DSP_DCBLOCK_H
#define DSP_DCBLOCK_H
#include "dsp_types.h"
#if DSP_ENABLE_DCBLOCK
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_f32_t r;     /* ضریب بازخورد (0..1) */
    dsp_f32_t x1;
    dsp_f32_t y1;
} dsp_dcblock_f32_t;

dsp_err_t dsp_dcblock_init_f32(dsp_dcblock_f32_t *p, dsp_f32_t r);
dsp_err_t dsp_dcblock_process_f32(dsp_dcblock_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_dcblock_process_block_f32(dsp_dcblock_f32_t *p, const dsp_f32_t *src,
                                        dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_dcblock_reset_f32(dsp_dcblock_f32_t *p);

#if DSP_USE_SHORT_ALIASES
  #define DCBlock_Init    dsp_dcblock_init_f32
  #define DCBlock_Process dsp_dcblock_process_f32
#endif

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_DCBLOCK */
#endif
