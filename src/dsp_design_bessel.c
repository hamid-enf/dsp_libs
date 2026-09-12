/**
 ******************************************************************************
 * @file    dsp_design_bessel.c
 * @brief   طراحی فیلتر Bessel (Bessel-Thomson) — LP/HP/BP/BS
 *
 * چندجمله‌ای بسل:
 *   y_N(s) = Σ_{k=0..N}  ((N+k)! / ((N−k)!·k!·2^k)) · s^k
 *
 * قطب‌ها: ریشه‌های چندجمله‌ای بسل (روش Durand-Kerner) معکوس می‌شوند و با
 * نرمال‌سازی فاز (مطابق MATLAB/scipy):
 *   a_last = (2N)!/(N!·2^N) ،  p ← p · 10^(−log10(a_last)/N)
 * این نرمال‌سازی باعث می‌شود فاز در ω=1 به نصف بیشینه برسد (Phase-matched)
 * و در نتیجه Group-Delay در پاس‌باند تقریباً مسطح باشد.
 *
 * ویژگی: بدون Overshoot در پاسخ پله و Group-Delay یکنواخت — برای کاربردهای
 * حساس به فاز (فیلتر کراس‌اوور، همگام‌سازی) ایده‌آل است.
 */
#include "dsp_config.h"

#if DSP_ENABLE_BESSEL

#include "dsp_design.h"
#include "dsp_design_int.h"
#include "dsp_math.h"



dsp_err_t dsp_design_bessel(const dsp_design_params_t *cfg)
{
    int n;
    double coeffs[DSP_MAX_DESIGN_ORDER + 1];
    dsp_cx_t roots[DSP_MAX_DESIGN_ORDER];
    dsp_cx_t p[DSP_MAX_DESIGN_ORDER];
    double a_last = 1.0, scale;
    dsp_err_t err;
    int i;

    if (cfg == NULL) return DSP_ERR_NULL_PTR;
    n = cfg->order;
    if (n < 1 || n > DSP_MAX_DESIGN_ORDER) return DSP_ERR_INVALID_PARAMETER;

    dsp_bessel_poly_coeffs(n, coeffs);
    /* dsp_poly_roots ضرایب را با بالاترین توان اول می‌گیرد؛ معکوس کن */
    {
        double pdesc[DSP_MAX_DESIGN_ORDER + 1];
        for (i = 0; i <= n; i++) pdesc[i] = coeffs[n - i];
        err = dsp_poly_roots(pdesc, n, roots);
        if (err != DSP_OK) return err;
    }

    /* a_last = (2N)!/(N!·2^N) — از ضرایب بسل: c_N = (2N)!/(N!·2^N) */
    a_last = coeffs[n];

    /* p = 1/root سپس نرمال‌سازی فاز */
    scale = pow(10.0, -log10(a_last) / (double)n);
    for (i = 0; i < n; i++) {
        dsp_cx_t inv = dsp_cx_div(dsp_cx_make(1.0, 0.0), roots[i]);
        p[i] = dsp_cx_scale(inv, scale);
    }

    return dsp_design_run(cfg, p, n, NULL, 0, 1.0, 1.0);
}

#endif /* DSP_ENABLE_BESSEL */
