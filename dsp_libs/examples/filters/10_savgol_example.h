/*
 * 10_savgol_example.h — Savitzky-Golay
 *
 * کاربرد: smoothing با حفظ قله و شکل موج، اندازه‌گیری و تخمین مشتق. در صوت
 * Real-Time تأخیر M نمونه دارد؛ طراحی ضرایب فقط در init انجام می‌شود.
 */
#ifndef DSP_EXAMPLE_10_SAVGOL_H
#define DSP_EXAMPLE_10_SAVGOL_H

#include "dsp.h"

#if DSP_ENABLE_SAVGOL
#define DSP_EXAMPLE_SG_M 3u
#define DSP_EXAMPLE_SG_POINTS (2u * DSP_EXAMPLE_SG_M + 1u)
static dsp_f32_t dsp_example_sg_coeffs[DSP_EXAMPLE_SG_POINTS];
static dsp_f32_t dsp_example_sg_buf[DSP_EXAMPLE_SG_POINTS];
static dsp_savgol_f32_t dsp_example_sg;

static inline dsp_err_t dsp_example_savgol_init(void)
{
    dsp_err_t e = dsp_savgol_design_f32(DSP_EXAMPLE_SG_M, 2u, 0u,
                                         dsp_example_sg_coeffs);
    if (e != DSP_OK) return e;
    return dsp_savgol_init_f32(&dsp_example_sg, DSP_EXAMPLE_SG_M,
                               dsp_example_sg_coeffs, dsp_example_sg_buf);
}

static inline dsp_err_t dsp_example_savgol_process(dsp_f32_t input,
                                                    dsp_f32_t *output)
{
    return dsp_savgol_process_f32(&dsp_example_sg, input, output);
}
#endif /* DSP_ENABLE_SAVGOL */
#endif /* DSP_EXAMPLE_10_SAVGOL_H */
