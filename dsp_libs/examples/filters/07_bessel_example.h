/*
 * 07_bessel_example.h — Bessel
 *
 * کاربرد: صدا و اندازه‌گیری‌هایی که شکل موج/Group Delay مهم‌تر از شیب قطع
 * است؛ گذرای نرم و phase تقریباً خطی دارد، اما نسبت به Butterworth شیب کمتری.
 */
#ifndef DSP_EXAMPLE_07_BESSEL_H
#define DSP_EXAMPLE_07_BESSEL_H

#include "dsp.h"

#if DSP_ENABLE_BESSEL && DSP_ENABLE_IIR
#define DSP_EXAMPLE_BESSEL_ORDER 4u
#define DSP_EXAMPLE_BESSEL_SECTIONS (DSP_EXAMPLE_BESSEL_ORDER / 2u)
static dsp_f64_t dsp_example_bessel_sos64[5u * DSP_EXAMPLE_BESSEL_SECTIONS];
static dsp_f32_t dsp_example_bessel_sos32[5u * DSP_EXAMPLE_BESSEL_SECTIONS];
static dsp_f32_t dsp_example_bessel_state[2u * DSP_EXAMPLE_BESSEL_SECTIONS];
static dsp_iir_f32_t dsp_example_bessel_iir;

static inline dsp_err_t dsp_example_bessel_init(uint32_t sample_rate,
                                                 dsp_f64_t cutoff_hz)
{
    dsp_design_params_t cfg = {0};
    uint16_t sections = 0u;
    dsp_err_t e;
    if (sample_rate == 0u || cutoff_hz <= 0.0 || cutoff_hz >= (dsp_f64_t)sample_rate * 0.5) return DSP_ERR_INVALID_PARAMETER;
    cfg.type = DSP_FILTER_LP; cfg.order = DSP_EXAMPLE_BESSEL_ORDER;
    cfg.fs = (dsp_f64_t)sample_rate; cfg.fc = cutoff_hz;
    cfg.pSOS = dsp_example_bessel_sos64; cfg.pNumSections = &sections;
    e = dsp_design_bessel(&cfg);
    if (e != DSP_OK) return e;
    if (sections != DSP_EXAMPLE_BESSEL_SECTIONS) return DSP_ERR_INVALID_STATE;
    dsp_design_sos_f64_to_f32(dsp_example_bessel_sos64, sections, dsp_example_bessel_sos32);
    return dsp_iir_init_f32(&dsp_example_bessel_iir, DSP_TDF2,
                            DSP_EXAMPLE_BESSEL_ORDER, dsp_example_bessel_sos32,
                            dsp_example_bessel_state,
                            (uint32_t)(sizeof dsp_example_bessel_state / sizeof dsp_example_bessel_state[0]), 1u);
}

static inline dsp_err_t dsp_example_bessel_process(dsp_f32_t input,
                                                    dsp_f32_t *output)
{
    return dsp_iir_process_sample_f32(&dsp_example_bessel_iir, input, output);
}
#endif /* DSP_ENABLE_BESSEL && DSP_ENABLE_IIR */
#endif /* DSP_EXAMPLE_07_BESSEL_H */
