/*
 * 06_elliptic_example.h — Elliptic/Cauer
 *
 * کاربرد: وقتی با کمترین مرتبه، transition بسیار تند لازم است؛ در عوض هم در
 * passband و هم stopband ripple دارد و به تغییرات عددی حساس‌تر است.
 * این طراحی را روی target در boot قابل انجام است، اما برای صدای محصولی بهتر
 * است SOS را Offline تولید و در Flash ذخیره کنید.
 */
#ifndef DSP_EXAMPLE_06_ELLIPTIC_H
#define DSP_EXAMPLE_06_ELLIPTIC_H

#include "dsp.h"

#if DSP_ENABLE_ELLIPTIC && DSP_ENABLE_IIR
#define DSP_EXAMPLE_ELLIPTIC_ORDER 4u
#define DSP_EXAMPLE_ELLIPTIC_SECTIONS (DSP_EXAMPLE_ELLIPTIC_ORDER / 2u)
static dsp_f64_t dsp_example_elliptic_sos64[5u * DSP_EXAMPLE_ELLIPTIC_SECTIONS];
static dsp_f32_t dsp_example_elliptic_sos32[5u * DSP_EXAMPLE_ELLIPTIC_SECTIONS];
static dsp_f32_t dsp_example_elliptic_state[2u * DSP_EXAMPLE_ELLIPTIC_SECTIONS];
static dsp_iir_f32_t dsp_example_elliptic_iir;

static inline dsp_err_t dsp_example_elliptic_init(uint32_t sample_rate,
                                                   dsp_f64_t cutoff_hz)
{
    dsp_design_params_t cfg = {0};
    uint16_t sections = 0u;
    dsp_err_t e;
    if (sample_rate == 0u || cutoff_hz <= 0.0 || cutoff_hz >= (dsp_f64_t)sample_rate * 0.5) return DSP_ERR_INVALID_PARAMETER;
    cfg.type = DSP_FILTER_LP; cfg.order = DSP_EXAMPLE_ELLIPTIC_ORDER;
    cfg.fs = (dsp_f64_t)sample_rate; cfg.fc = cutoff_hz;
    cfg.ripple_db = 0.5; cfg.stop_db = 60.0;
    cfg.pSOS = dsp_example_elliptic_sos64; cfg.pNumSections = &sections;
    e = dsp_design_elliptic(&cfg);
    if (e != DSP_OK) return e;
    if (sections != DSP_EXAMPLE_ELLIPTIC_SECTIONS) return DSP_ERR_INVALID_STATE;
    dsp_design_sos_f64_to_f32(dsp_example_elliptic_sos64, sections, dsp_example_elliptic_sos32);
    return dsp_iir_init_f32(&dsp_example_elliptic_iir, DSP_TDF2,
                            DSP_EXAMPLE_ELLIPTIC_ORDER, dsp_example_elliptic_sos32,
                            dsp_example_elliptic_state,
                            (uint32_t)(sizeof dsp_example_elliptic_state / sizeof dsp_example_elliptic_state[0]), 1u);
}

static inline dsp_err_t dsp_example_elliptic_process(dsp_f32_t input,
                                                      dsp_f32_t *output)
{
    return dsp_iir_process_sample_f32(&dsp_example_elliptic_iir, input, output);
}
#endif /* DSP_ENABLE_ELLIPTIC && DSP_ENABLE_IIR */
#endif /* DSP_EXAMPLE_06_ELLIPTIC_H */
