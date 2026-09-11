/*
 * 14_kalman_example.h — Kalman یک‌بعدی
 *
 * کاربرد: نرم‌کردن position/temperature/level و ترکیب اندازه‌گیری noisy با
 * مدل ساده‌ی ثابت. Q بزرگ‌تر یعنی اعتماد بیشتر به تغییرات واقعی؛ R بزرگ‌تر
 * یعنی اعتماد کمتر به سنسور. برای هر کمیت یا کانال یک instance جدا بسازید.
 */
#ifndef DSP_EXAMPLE_14_KALMAN_H
#define DSP_EXAMPLE_14_KALMAN_H

#include "dsp.h"

#if DSP_ENABLE_KALMAN
static dsp_kalman1d_f32_t dsp_example_kalman;

static inline dsp_err_t dsp_example_kalman_init(dsp_f32_t initial_value)
{
    return dsp_kalman1d_init_f32(&dsp_example_kalman,
                                 initial_value,
                                 1.0f,    /* p0 */
                                 1e-3f,   /* Q: نویز فرآیند */
                                 0.05f);  /* R: نویز اندازه‌گیری */
}

static inline dsp_err_t dsp_example_kalman_process(dsp_f32_t measurement,
                                                    dsp_f32_t *estimate)
{
    return dsp_kalman1d_process_f32(&dsp_example_kalman, measurement, estimate);
}
#endif /* DSP_ENABLE_KALMAN */
#endif /* DSP_EXAMPLE_14_KALMAN_H */
