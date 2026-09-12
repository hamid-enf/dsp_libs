/*
 * 01_fir_example.h — مثال عملی FIR برای صدای Real-Time
 *
 * کاربرد: Low-Pass سبک برای حذف نویز فرکانس بالا، smoothing سنسور و anti-noise.
 * نکته: coeffs و state باید تا پایان عمر فیلتر زنده بمانند؛ در callback هرگز
 * init/reset را صدا نزنید، چون حافظه‌ی حالت پاک می‌شود.
 */
#ifndef DSP_EXAMPLE_01_FIR_H
#define DSP_EXAMPLE_01_FIR_H

#include "dsp.h"

#if DSP_ENABLE_FIR
#define DSP_EXAMPLE_FIR_TAPS   5u
#define DSP_EXAMPLE_FIR_BLOCK  128u

static const dsp_f32_t dsp_example_fir_coeffs[DSP_EXAMPLE_FIR_TAPS] = {
    0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f
};
static dsp_f32_t dsp_example_fir_state[DSP_EXAMPLE_FIR_TAPS + DSP_EXAMPLE_FIR_BLOCK - 1u];
static dsp_fir_f32_t dsp_example_fir;

/* یک‌بار در init پروژه، بعد از مشخص شدن اندازه‌ی بزرگ‌ترین بلاک. */
static inline dsp_err_t dsp_example_fir_init(void)
{
    return dsp_fir_init_f32(&dsp_example_fir,
                            dsp_example_fir_coeffs,
                            DSP_EXAMPLE_FIR_TAPS,
                            DSP_EXAMPLE_FIR_BLOCK,
                            DSP_FORM_DIRECT,
                            dsp_example_fir_state,
                            (uint32_t)(sizeof dsp_example_fir_state / sizeof dsp_example_fir_state[0]),
                            0u);
}

/* مناسب برای callback نمونه‌به‌نمونه؛ state بین callbackها حفظ می‌شود. */
static inline dsp_err_t dsp_example_fir_process_sample(dsp_f32_t input,
                                                        dsp_f32_t *output)
{
    return dsp_fir_process_sample_f32(&dsp_example_fir, input, output);
}

/* مناسب برای callbackهای DMA؛ n نباید از DSP_EXAMPLE_FIR_BLOCK بزرگ‌تر باشد. */
static inline dsp_err_t dsp_example_fir_process_block(const dsp_f32_t *input,
                                                       dsp_f32_t *output,
                                                       uint16_t n)
{
    return dsp_fir_process_block_f32(&dsp_example_fir, input, output, n);
}

/* اگر ترک یا مسیر سیگنال عوض شد و می‌خواهید فیلتر از صفر شروع کند. */
static inline dsp_err_t dsp_example_fir_reset(void)
{
    return dsp_fir_reset_f32(&dsp_example_fir);
}
#endif /* DSP_ENABLE_FIR */
#endif /* DSP_EXAMPLE_01_FIR_H */
