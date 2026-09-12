/*
 * 11_dc_blocker_example.h — حذف DC از ADC/میکروفون
 *
 * کاربرد: حذف offset میکروفون یا bias ADC قبل از EQ/FFT. R=.995 برای صوت
 * معمولی نقطه‌ی شروع خوبی است؛ R بزرگ‌تر cutoff پایین‌تری می‌دهد اما زمان
 * نشست را بیشتر می‌کند.
 */
#ifndef DSP_EXAMPLE_11_DCBLOCK_H
#define DSP_EXAMPLE_11_DCBLOCK_H

#include "dsp.h"

#if DSP_ENABLE_DCBLOCK
static dsp_dcblock_f32_t dsp_example_dcblock;

static inline dsp_err_t dsp_example_dcblock_init(void)
{
    return dsp_dcblock_init_f32(&dsp_example_dcblock, 0.995f);
}

static inline dsp_err_t dsp_example_dcblock_process(dsp_f32_t input,
                                                     dsp_f32_t *output)
{
    return dsp_dcblock_process_f32(&dsp_example_dcblock, input, output);
}

static inline dsp_err_t dsp_example_dcblock_process_block(const dsp_f32_t *input,
                                                          dsp_f32_t *output,
                                                          uint16_t n)
{
    return dsp_dcblock_process_block_f32(&dsp_example_dcblock, input, output, n);
}
#endif /* DSP_ENABLE_DCBLOCK */
#endif /* DSP_EXAMPLE_11_DCBLOCK_H */
