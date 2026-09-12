/*
 * 13_adaptive_example.h — LMS/NLMS و حذف نویز دو میکروفون
 *
 * d = میکروفون اصلی (speech + noise)، x = میکروفون مرجع (noise).
 * خروجی e تقریباً گفتار تمیز است. هم‌زمانی و alignment دو ورودی بسیار مهم
 * است. mu را کوچک شروع کنید؛ برای توان متغیر NLMS معمولاً مناسب‌تر است.
 */
#ifndef DSP_EXAMPLE_13_ADAPTIVE_H
#define DSP_EXAMPLE_13_ADAPTIVE_H

#include "dsp.h"

#if (DSP_ENABLE_LMS || DSP_ENABLE_NLMS)
#define DSP_EXAMPLE_ADAPTIVE_LEN 32u
static dsp_f32_t dsp_example_lms_w[DSP_EXAMPLE_ADAPTIVE_LEN];
static dsp_f32_t dsp_example_lms_x[DSP_EXAMPLE_ADAPTIVE_LEN];
static dsp_f32_t dsp_example_nlms_w[DSP_EXAMPLE_ADAPTIVE_LEN];
static dsp_f32_t dsp_example_nlms_x[DSP_EXAMPLE_ADAPTIVE_LEN];
static dsp_lms_f32_t dsp_example_lms;
static dsp_nlms_f32_t dsp_example_nlms;

static inline dsp_err_t dsp_example_adaptive_init(void)
{
    dsp_err_t e = DSP_OK;
#if DSP_ENABLE_LMS
    e = dsp_lms_init_f32(&dsp_example_lms, DSP_EXAMPLE_ADAPTIVE_LEN,
                         0.003f, 0.0f, dsp_example_lms_w, dsp_example_lms_x);
    if (e != DSP_OK) return e;
#endif
#if DSP_ENABLE_NLMS
    e = dsp_nlms_init_f32(&dsp_example_nlms, DSP_EXAMPLE_ADAPTIVE_LEN,
                          0.25f, 1e-6f, dsp_example_nlms_w, dsp_example_nlms_x);
#endif
    return e;
}

#if DSP_ENABLE_LMS
static inline dsp_err_t dsp_example_lms_process(dsp_f32_t reference_noise,
                                                 dsp_f32_t microphone,
                                                 dsp_f32_t *estimated_noise,
                                                 dsp_f32_t *clean)
{
    return dsp_lms_process_f32(&dsp_example_lms, reference_noise, microphone,
                               estimated_noise, clean);
}
#endif

#if DSP_ENABLE_NLMS
static inline dsp_err_t dsp_example_nlms_process(dsp_f32_t reference_noise,
                                                  dsp_f32_t microphone,
                                                  dsp_f32_t *estimated_noise,
                                                  dsp_f32_t *clean)
{
    return dsp_nlms_process_f32(&dsp_example_nlms, reference_noise, microphone,
                                estimated_noise, clean);
}
#endif
#endif /* LMS || NLMS */
#endif /* DSP_EXAMPLE_13_ADAPTIVE_H */
