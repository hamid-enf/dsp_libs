/*
 * 08_moving_average_example.h — SMA / WMA / EMA
 *
 * کاربرد: smoothing خوانش حسگر، کنترل آهسته و VU meter. SMA/WMA پنجره و
 * تأخیر دارند؛ EMA حافظه‌ی بسیار کم و پاسخ سریع‌تری دارد.
 */
#ifndef DSP_EXAMPLE_08_MOVING_H
#define DSP_EXAMPLE_08_MOVING_H

#include "dsp.h"

#if DSP_ENABLE_MOVING_AVERAGE
#define DSP_EXAMPLE_MOVING_WINDOW 8u
static dsp_f32_t dsp_example_sma_buf[DSP_EXAMPLE_MOVING_WINDOW];
static dsp_f32_t dsp_example_wma_buf[DSP_EXAMPLE_MOVING_WINDOW];
static const dsp_f32_t dsp_example_wma_weights[DSP_EXAMPLE_MOVING_WINDOW] = {
    1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f
};
static dsp_sma_f32_t dsp_example_sma;
static dsp_wma_f32_t dsp_example_wma;
static dsp_ema_f32_t dsp_example_ema;

static inline dsp_err_t dsp_example_moving_init(void)
{
    dsp_err_t e = dsp_sma_init_f32(&dsp_example_sma, DSP_EXAMPLE_MOVING_WINDOW,
                                   dsp_example_sma_buf);
    if (e != DSP_OK) return e;
    e = dsp_wma_init_f32(&dsp_example_wma, DSP_EXAMPLE_MOVING_WINDOW,
                         dsp_example_wma_weights, dsp_example_wma_buf);
    if (e != DSP_OK) return e;
    return dsp_ema_init_f32(&dsp_example_ema, 0.10f);
}

static inline dsp_err_t dsp_example_sma_process(dsp_f32_t input, dsp_f32_t *output)
{
    return dsp_sma_process_f32(&dsp_example_sma, input, output);
}
static inline dsp_err_t dsp_example_wma_process(dsp_f32_t input, dsp_f32_t *output)
{
    return dsp_wma_process_f32(&dsp_example_wma, input, output);
}
static inline dsp_err_t dsp_example_ema_process(dsp_f32_t input, dsp_f32_t *output)
{
    return dsp_ema_process_f32(&dsp_example_ema, input, output);
}
#endif /* DSP_ENABLE_MOVING_AVERAGE */
#endif /* DSP_EXAMPLE_08_MOVING_H */
