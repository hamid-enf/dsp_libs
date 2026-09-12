/*
 * 02_iir_example.h — مثال IIR مرتبه‌ی ۲ با فرم TDF2
 *
 * کاربرد: فیلتر صوتی کم‌هزینه با حافظه‌ی کم. برای مرتبه‌ی بالا، به‌جای یک
 * IIR مستقیم، ضرایب SOS را با یکی از headerهای طراحی بسازید و cascade کنید.
 * این ضرایب نمونه، Butterworth Low-Pass مرتبه‌ی ۲ با fc=1kHz و Fs=48kHz
 * هستند و با قرارداد a0=1 ذخیره شده‌اند.
 */
#ifndef DSP_EXAMPLE_02_IIR_H
#define DSP_EXAMPLE_02_IIR_H

#include "dsp.h"

#if DSP_ENABLE_IIR
#define DSP_EXAMPLE_IIR_ORDER 2u

/* چیدمان مستقیم: [b0 ... bN | a1 ... aN] و a0=1. */
static const dsp_f32_t dsp_example_iir_coeffs[2u * DSP_EXAMPLE_IIR_ORDER + 1u] = {
    0.0039161267f, 0.0078322533f, 0.0039161267f,
   -1.8153410827f, 0.8310055893f
};
static dsp_f32_t dsp_example_iir_state[ DSP_EXAMPLE_IIR_ORDER ];
static dsp_iir_f32_t dsp_example_iir;

static inline dsp_err_t dsp_example_iir_init(void)
{
    /* TDF2 برای نمونه‌ی float32 کم‌حافظه و پایدارتر از DF1 است. */
    return dsp_iir_init_f32(&dsp_example_iir,
                            DSP_TDF2,
                            DSP_EXAMPLE_IIR_ORDER,
                            dsp_example_iir_coeffs,
                            dsp_example_iir_state,
                            (uint32_t)(sizeof dsp_example_iir_state / sizeof dsp_example_iir_state[0]),
                            0u);
}

static inline dsp_err_t dsp_example_iir_process_sample(dsp_f32_t input,
                                                        dsp_f32_t *output)
{
    return dsp_iir_process_sample_f32(&dsp_example_iir, input, output);
}

static inline dsp_err_t dsp_example_iir_process_block(const dsp_f32_t *input,
                                                       dsp_f32_t *output,
                                                       uint16_t n)
{
    return dsp_iir_process_block_f32(&dsp_example_iir, input, output, n);
}

static inline dsp_err_t dsp_example_iir_reset(void)
{
    return dsp_iir_reset_f32(&dsp_example_iir);
}
#endif /* DSP_ENABLE_IIR */
#endif /* DSP_EXAMPLE_02_IIR_H */
