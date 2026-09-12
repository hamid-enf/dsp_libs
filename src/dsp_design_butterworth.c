/**
 * @file dsp_design_butterworth.c
 * @brief طراحی فیلتر Butterworth — LP/HP/BP/BS
 *
 * قطب‌های پروتوتایپ آنالوگ (نرمال‌شده، قطع = 1):
 *   p_k = -exp(j·π·m_k / (2N)) ،  m_k = -(N-1), -(N-3), ..., (N-1)
 * تبدیل فرکانسی + Bilinear (T=2) + نرمال‌سازی گین DC = 1.
 */
#include "dsp_config.h"

#if DSP_ENABLE_BUTTERWORTH

#include "dsp_design.h"
#include "dsp_design_int.h"
#include "dsp_math.h"



dsp_err_t dsp_design_butterworth(const dsp_design_params_t *cfg)
{
    dsp_cx_t p[DSP_MAX_DESIGN_ORDER];
    int n, m, idx = 0;
    if (cfg == NULL) return DSP_ERR_NULL_PTR;
    n = cfg->order;
    if (n < 1 || n > DSP_MAX_DESIGN_ORDER) return DSP_ERR_INVALID_PARAMETER;

    for (m = -n + 1; m <= n - 1; m += 2) {
        double th = DSP_PI * (double)m / (2.0 * (double)n);
        p[idx++] = dsp_cx_make(-cos(th), -sin(th));
    }
    return dsp_design_run(cfg, p, n, NULL, 0, 1.0, 1.0);
}

#endif /* DSP_ENABLE_BUTTERWORTH */
