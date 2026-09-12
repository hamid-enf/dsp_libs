/*
 * 12_notch_example.h — حذف برق شهر 50Hz/60Hz
 *
 * کاربرد: میکروفون/ADC با hum برق. در سیستم‌هایی که فرکانس شبکه قابل تغییر
 * است، dsp_notch_set_freq_f32 را از task کنترل (نه callback صوت) صدا بزنید.
 */
#ifndef DSP_EXAMPLE_12_NOTCH_H
#define DSP_EXAMPLE_12_NOTCH_H

#include "dsp.h"

#if DSP_ENABLE_NOTCH
static dsp_notch_f32_t dsp_example_notch;

static inline dsp_err_t dsp_example_notch_init(uint32_t sample_rate,
                                                dsp_f32_t mains_hz)
{
    return dsp_notch_init_f32(&dsp_example_notch, sample_rate, mains_hz, 20.0f);
}

static inline dsp_err_t dsp_example_notch_change_frequency(uint32_t sample_rate,
                                                            dsp_f32_t mains_hz)
{
    /* تغییر ضرایب بدون reset؛ برای تغییر آرام پارامتر، بیرون callback انجام شود. */
    return dsp_notch_set_freq_f32(&dsp_example_notch, sample_rate, mains_hz, 20.0f);
}

static inline dsp_err_t dsp_example_notch_process(dsp_f32_t input,
                                                   dsp_f32_t *output)
{
    return dsp_notch_process_f32(&dsp_example_notch, input, output);
}
#endif /* DSP_ENABLE_NOTCH */
#endif /* DSP_EXAMPLE_12_NOTCH_H */
