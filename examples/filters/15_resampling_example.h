/*
 * 15_resampling_example.h — Interpolator ×2 و Decimator ÷2
 *
 * کاربرد: تغییر نرخ بین audio streams، decimation سنسور و interpolation.
 * تابع decim برای nOut خروجی، d*nOut ورودی مصرف می‌کند؛ interp برای nIn
 * ورودی، l*nIn خروجی می‌دهد. ضرایب فیلتر anti-alias در init ساخته می‌شوند.
 */
#ifndef DSP_EXAMPLE_15_RESAMPLING_H
#define DSP_EXAMPLE_15_RESAMPLING_H

#include "dsp.h"

#if DSP_ENABLE_RESAMPLING
#define DSP_EXAMPLE_RESAMPLE_FACTOR 2u
#define DSP_EXAMPLE_RESAMPLE_PHASE_TAPS 4u
#define DSP_EXAMPLE_RESAMPLE_TAPS (DSP_EXAMPLE_RESAMPLE_FACTOR * DSP_EXAMPLE_RESAMPLE_PHASE_TAPS)

static dsp_f32_t dsp_example_decim_coeffs[DSP_EXAMPLE_RESAMPLE_TAPS];
static dsp_f32_t dsp_example_decim_state[DSP_EXAMPLE_RESAMPLE_TAPS];
static dsp_f32_t dsp_example_interp_coeffs[DSP_EXAMPLE_RESAMPLE_TAPS];
static dsp_f32_t dsp_example_interp_state[DSP_EXAMPLE_RESAMPLE_PHASE_TAPS];
static dsp_decim_f32_t dsp_example_decim;
static dsp_interp_f32_t dsp_example_interp;

static inline dsp_err_t dsp_example_resampling_init(void)
{
    dsp_err_t e;
    /* cutoff نرمال‌شده نسبت به نرخ ورودی؛ برای نمونه‌ی آموزشی. */
    dsp_resample_design_lp_f32(dsp_example_decim_coeffs,
                               DSP_EXAMPLE_RESAMPLE_TAPS, 0.20f);
    dsp_resample_design_lp_f32(dsp_example_interp_coeffs,
                               DSP_EXAMPLE_RESAMPLE_TAPS, 0.20f);
    e = dsp_decim_init_f32(&dsp_example_decim,
                           DSP_EXAMPLE_RESAMPLE_FACTOR,
                           DSP_EXAMPLE_RESAMPLE_PHASE_TAPS,
                           dsp_example_decim_coeffs,
                           dsp_example_decim_state);
    if (e != DSP_OK) return e;
    return dsp_interp_init_f32(&dsp_example_interp,
                               DSP_EXAMPLE_RESAMPLE_FACTOR,
                               DSP_EXAMPLE_RESAMPLE_PHASE_TAPS,
                               dsp_example_interp_coeffs,
                               dsp_example_interp_state);
}

static inline dsp_err_t dsp_example_decim_process(const dsp_f32_t *input,
                                                   dsp_f32_t *output,
                                                   uint16_t output_count)
{
    return dsp_decim_process_f32(&dsp_example_decim, input, output, output_count);
}

static inline dsp_err_t dsp_example_interp_process(const dsp_f32_t *input,
                                                    dsp_f32_t *output,
                                                    uint16_t input_count)
{
    return dsp_interp_process_f32(&dsp_example_interp, input, output, input_count);
}
#endif /* DSP_ENABLE_RESAMPLING */
#endif /* DSP_EXAMPLE_15_RESAMPLING_H */
