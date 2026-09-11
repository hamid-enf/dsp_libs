/*
 * 05_chebyshev_example.h — Chebyshev نوع I و II
 *
 * نوع I: transition band باریک‌تر با ripple در passband.
 * نوع II: passband صاف‌تر ولی ripple/صفرها در stopband.
 * هر دو خروجی SOS هستند و فقط در init طراحی می‌شوند.
 */
#ifndef DSP_EXAMPLE_05_CHEBYSHEV_H
#define DSP_EXAMPLE_05_CHEBYSHEV_H

#include "dsp.h"

#if DSP_ENABLE_CHEBYSHEV && DSP_ENABLE_IIR
#define DSP_EXAMPLE_CHEB_ORDER 4u
#define DSP_EXAMPLE_CHEB_SECTIONS (DSP_EXAMPLE_CHEB_ORDER / 2u)
static dsp_f64_t dsp_example_cheb1_sos64[5u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_f64_t dsp_example_cheb2_sos64[5u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_f32_t dsp_example_cheb1_sos32[5u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_f32_t dsp_example_cheb2_sos32[5u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_f32_t dsp_example_cheb1_state[2u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_f32_t dsp_example_cheb2_state[2u * DSP_EXAMPLE_CHEB_SECTIONS];
static dsp_iir_f32_t dsp_example_cheb1_iir;
static dsp_iir_f32_t dsp_example_cheb2_iir;

static inline dsp_err_t dsp_example_chebyshev_init(uint32_t sample_rate,
                                                    dsp_f64_t cutoff_hz)
{
    dsp_design_params_t c1 = {0};
    dsp_design_params_t c2 = {0};
    uint16_t n1 = 0u, n2 = 0u;
    dsp_err_t e;
    if (sample_rate == 0u || cutoff_hz <= 0.0 || cutoff_hz >= (dsp_f64_t)sample_rate * 0.5) {
        return DSP_ERR_INVALID_PARAMETER;
    }
    c1.type = DSP_FILTER_LP; c1.order = DSP_EXAMPLE_CHEB_ORDER;
    c1.fs = (dsp_f64_t)sample_rate; c1.fc = cutoff_hz; c1.ripple_db = 0.5;
    c1.pSOS = dsp_example_cheb1_sos64; c1.pNumSections = &n1;
    e = dsp_design_chebyshev1(&c1); if (e != DSP_OK) return e;

    c2.type = DSP_FILTER_LP; c2.order = DSP_EXAMPLE_CHEB_ORDER;
    c2.fs = (dsp_f64_t)sample_rate; c2.fc = cutoff_hz; c2.stop_db = 50.0;
    c2.pSOS = dsp_example_cheb2_sos64; c2.pNumSections = &n2;
    e = dsp_design_chebyshev2(&c2); if (e != DSP_OK) return e;
    if (n1 != DSP_EXAMPLE_CHEB_SECTIONS || n2 != DSP_EXAMPLE_CHEB_SECTIONS) return DSP_ERR_INVALID_STATE;

    dsp_design_sos_f64_to_f32(dsp_example_cheb1_sos64, n1, dsp_example_cheb1_sos32);
    dsp_design_sos_f64_to_f32(dsp_example_cheb2_sos64, n2, dsp_example_cheb2_sos32);
    e = dsp_iir_init_f32(&dsp_example_cheb1_iir, DSP_TDF2, DSP_EXAMPLE_CHEB_ORDER,
                         dsp_example_cheb1_sos32, dsp_example_cheb1_state,
                         (uint32_t)(sizeof dsp_example_cheb1_state / sizeof dsp_example_cheb1_state[0]), 1u);
    if (e != DSP_OK) return e;
    return dsp_iir_init_f32(&dsp_example_cheb2_iir, DSP_TDF2, DSP_EXAMPLE_CHEB_ORDER,
                            dsp_example_cheb2_sos32, dsp_example_cheb2_state,
                            (uint32_t)(sizeof dsp_example_cheb2_state / sizeof dsp_example_cheb2_state[0]), 1u);
}

/* خروجی نوع I و نوع II جداست؛ هر دو یک ورودی مشترک می‌گیرند. */
static inline dsp_err_t dsp_example_chebyshev_process(dsp_f32_t input,
                                                       dsp_f32_t *type1_output,
                                                       dsp_f32_t *type2_output)
{
    dsp_err_t e;
    if (type1_output == NULL || type2_output == NULL) return DSP_ERR_NULL_PTR;
    e = dsp_iir_process_sample_f32(&dsp_example_cheb1_iir, input, type1_output);
    if (e != DSP_OK) return e;
    return dsp_iir_process_sample_f32(&dsp_example_cheb2_iir, input, type2_output);
}
#endif /* DSP_ENABLE_CHEBYSHEV && DSP_ENABLE_IIR */
#endif /* DSP_EXAMPLE_05_CHEBYSHEV_H */
