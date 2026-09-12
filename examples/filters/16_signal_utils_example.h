/*
 * 16_signal_utils_example.h — utilityهای صوتی: gain، RMS، envelope، limiter
 *
 * کاربرد: مرحله‌ی نهایی قبل از DAC/اسپیکر، VU meter و جلوگیری از clipping.
 * limiter را یک‌بار init کنید و state آن را بین بلاک‌ها حفظ کنید.
 */
#ifndef DSP_EXAMPLE_16_UTILS_H
#define DSP_EXAMPLE_16_UTILS_H

#include "dsp.h"

#if DSP_ENABLE_SIGNAL_UTILS
static dsp_limiter_f32_t dsp_example_limiter;
static dsp_env_f32_t dsp_example_envelope;

static inline dsp_err_t dsp_example_utils_init(void)
{
    dsp_err_t e;
    e = dsp_limiter_init_f32(&dsp_example_limiter,
                             0.90f, 0.98f, 0.20f, 0.001f);
    if (e != DSP_OK) return e;
    dsp_env_init_f32(&dsp_example_envelope, 0.20f, 0.002f);
    return DSP_OK;
}

static inline dsp_err_t dsp_example_utils_process_sample(dsp_f32_t input,
                                                          dsp_f32_t *limited,
                                                          dsp_f32_t *envelope)
{
    dsp_err_t e;
    if (limited == NULL || envelope == NULL) return DSP_ERR_NULL_PTR;
    e = dsp_limiter_process_f32(&dsp_example_limiter, input, limited);
    if (e != DSP_OK) return e;
    return dsp_env_process_f32(&dsp_example_envelope, input, envelope);
}

static inline dsp_f32_t dsp_example_utils_rms(const dsp_f32_t *block,
                                               uint16_t count)
{
    return dsp_rms_f32(block, count);
}

static inline void dsp_example_utils_gain(const dsp_f32_t *input,
                                           dsp_f32_t *output,
                                           uint16_t count,
                                           dsp_f32_t gain)
{
    dsp_gain_f32(input, output, count, gain, 1u);
}
#endif /* DSP_ENABLE_SIGNAL_UTILS */
#endif /* DSP_EXAMPLE_16_UTILS_H */
