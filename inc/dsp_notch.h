/**
 * @file dsp_notch.h — Notch Filter (حذف یک فرکانس مشخص، مثلاً 50/60Hz)
 *
 * ساختار TDF2 با محاسبه‌ی ضرایب در زمان اجرا از (Fs, Fn, Q):
 *   w0 = 2π·Fn/Fs ، α = sin(w0)/(2Q)
 *   b = [1, -2cos(w0), 1] / (1+α)
 *   a = [-2cos(w0)/(1+α), (1-α)/(1+α)]
 */
#ifndef DSP_NOTCH_H
#define DSP_NOTCH_H
#include "dsp_types.h"
#if DSP_ENABLE_NOTCH
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_f32_t b0, b1, b2, a1, a2;
    dsp_f32_t d1, d2;     /* حالت TDF2 */
    uint32_t  fs;
    dsp_f32_t fn;         /* فرکانس ناچ */
    dsp_f32_t q;
} dsp_notch_f32_t;

dsp_err_t dsp_notch_init_f32(dsp_notch_f32_t *p, uint32_t fs, dsp_f32_t fn, dsp_f32_t q);
dsp_err_t dsp_notch_set_freq_f32(dsp_notch_f32_t *p, uint32_t fs, dsp_f32_t fn, dsp_f32_t q);
dsp_err_t dsp_notch_process_f32(dsp_notch_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_notch_process_block_f32(dsp_notch_f32_t *p, const dsp_f32_t *src,
                                      dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_notch_reset_f32(dsp_notch_f32_t *p);

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_NOTCH */
#endif
