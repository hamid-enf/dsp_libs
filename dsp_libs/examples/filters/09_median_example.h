/*
 * 09_median_example.h — Median فلو/اسپایک
 *
 * کاربرد: ADC، سنسور فاصله، جریان و دما وقتی چند نمونه‌ی پرت داریم. برای
 * صوت پیوسته معمولاً انتخاب اول نیست؛ چون O(window) است و transient را تغییر
 * می‌دهد. پنجره باید فرد باشد (مثلاً 5).
 */
#ifndef DSP_EXAMPLE_09_MEDIAN_H
#define DSP_EXAMPLE_09_MEDIAN_H

#include "dsp.h"

#if DSP_ENABLE_MEDIAN
#define DSP_EXAMPLE_MEDIAN_WINDOW 5u
static dsp_f32_t dsp_example_median_buf[DSP_EXAMPLE_MEDIAN_WINDOW];
static dsp_f32_t dsp_example_median_sorted[DSP_EXAMPLE_MEDIAN_WINDOW];
static dsp_median_f32_t dsp_example_median;
static dsp_q15_t dsp_example_median_q15_buf[DSP_EXAMPLE_MEDIAN_WINDOW];
static dsp_q15_t dsp_example_median_q15_sorted[DSP_EXAMPLE_MEDIAN_WINDOW];
static dsp_median_q15_t dsp_example_median_q15;

static inline dsp_err_t dsp_example_median_init(void)
{
    dsp_err_t e = dsp_median_init_f32(&dsp_example_median,
                                      DSP_EXAMPLE_MEDIAN_WINDOW,
                                      dsp_example_median_buf,
                                      dsp_example_median_sorted);
    if (e != DSP_OK) return e;
    return dsp_median_init_q15(&dsp_example_median_q15,
                               DSP_EXAMPLE_MEDIAN_WINDOW,
                               dsp_example_median_q15_buf,
                               dsp_example_median_q15_sorted);
}

static inline dsp_err_t dsp_example_median_process_f32(dsp_f32_t input,
                                                        dsp_f32_t *output)
{
    return dsp_median_process_f32(&dsp_example_median, input, output);
}

/* برای PCM/Q15 یا ADC نرمال‌شده در بازه‌ی -1..1. */
static inline dsp_err_t dsp_example_median_process_q15(dsp_q15_t input,
                                                        dsp_q15_t *output)
{
    return dsp_median_process_q15(&dsp_example_median_q15, input, output);
}
#endif /* DSP_ENABLE_MEDIAN */
#endif /* DSP_EXAMPLE_09_MEDIAN_H */
