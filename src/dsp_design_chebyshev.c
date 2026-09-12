/**
 * @file dsp_design_chebyshev.c
 * @brief طراحی فیلترهای Chebyshev Type I و Type II — LP/HP/BP/BS
 *
 * Chebyshev I (موج‌داری پاس‌باند):
 *   ε = sqrt(10^(rp/10) - 1) ،  a = asinh(1/ε)/N
 *   p_k = -sinh(a)·sin(θ_k) + j·cosh(a)·cos(θ_k) ،  θ_k = π(2k+1)/(2N)
 *   گین DC: مرتبه‌ی فرد = 1 ، مرتبه‌ی زوج = 10^(-rp/20)
 *
 * Chebyshev II (تضعیف استاپ‌باند؛ صفرهای موهومی):
 *   de = 1/sqrt(10^(rs/10) - 1) ،  mu = asinh(1/de)/N
 *   صفرها: z_k = j / sin(m·π/(2N))   (m فرد، بدون صفر در مبدأ)
 *   قطب‌ها: p_k = -1/sinh(mu + j·θ_k)
 */
#include "dsp_config.h"

#if DSP_ENABLE_CHEBYSHEV

#include "dsp_design.h"
#include "dsp_design_int.h"
#include "dsp_math.h"



static dsp_cx_t dsp_cx_sinh(dsp_cx_t a)
{
    return dsp_cx_make(sinh(a.re) * cos(a.im), cosh(a.re) * sin(a.im));
}

dsp_err_t dsp_design_chebyshev1(const dsp_design_params_t *cfg)
{
    dsp_cx_t p[DSP_MAX_DESIGN_ORDER];
    int n, k;
    double eps, mu, target;
    if (cfg == NULL) return DSP_ERR_NULL_PTR;
    n = cfg->order;
    if (n < 1 || n > DSP_MAX_DESIGN_ORDER) return DSP_ERR_INVALID_PARAMETER;
    if (cfg->ripple_db <= 0.0) return DSP_ERR_INVALID_PARAMETER;

    eps = sqrt(pow(10.0, 0.1 * cfg->ripple_db) - 1.0);
    mu  = asinh(1.0 / eps) / (double)n;

    for (k = 0; k < n; k++) {
        double th = DSP_PI * (2.0 * (double)k + 1.0) / (2.0 * (double)n);
        p[k] = dsp_cx_make(-sinh(mu) * sin(th), cosh(mu) * cos(th));
    }
    target = (n & 1) ? 1.0 : pow(10.0, -cfg->ripple_db / 20.0);
    return dsp_design_run(cfg, p, n, NULL, 0, dsp_prod_neg(p, n), target);
}

dsp_err_t dsp_design_chebyshev2(const dsp_design_params_t *cfg)
{
    dsp_cx_t p[DSP_MAX_DESIGN_ORDER];
    dsp_cx_t z[DSP_MAX_DESIGN_ORDER];
    int n, i, idxz = 0, idxp = 0;
    double de, mu, k;
    if (cfg == NULL) return DSP_ERR_NULL_PTR;
    n = cfg->order;
    if (n < 1 || n > DSP_MAX_DESIGN_ORDER) return DSP_ERR_INVALID_PARAMETER;
    if (cfg->stop_db <= 0.0) return DSP_ERR_INVALID_PARAMETER;

    de = 1.0 / sqrt(pow(10.0, 0.1 * cfg->stop_db) - 1.0);
    mu = asinh(1.0 / de) / (double)n;

    /* صفرها: m های فرد */
    if (n & 1) {
        for (i = -(n - 1); i <= -2; i += 2) {
            z[idxz++] = dsp_cx_make(0.0, 1.0 / sin(DSP_PI * (double)i / (2.0 * (double)n)));
        }
        for (i = 2; i <= n - 1; i += 2) {
            z[idxz++] = dsp_cx_make(0.0, 1.0 / sin(DSP_PI * (double)i / (2.0 * (double)n)));
        }
    } else {
        for (i = -(n - 1); i <= n - 1; i += 2) {
            z[idxz++] = dsp_cx_make(0.0, 1.0 / sin(DSP_PI * (double)i / (2.0 * (double)n)));
        }
    }
    /* قطب‌ها */
    for (i = -(n - 1); i <= n - 1; i += 2) {
        double th = DSP_PI * (double)i / (2.0 * (double)n);
        dsp_cx_t sh = dsp_cx_sinh(dsp_cx_make(mu, th));
        p[idxp++] = dsp_cx_scale(dsp_cx_conj(sh), -1.0 / (sh.re * sh.re + sh.im * sh.im));
    }
    k = dsp_prod_neg(p, n) / dsp_prod_neg(z, idxz);
    return dsp_design_run(cfg, p, n, z, idxz, k, 1.0);
}

#endif /* DSP_ENABLE_CHEBYSHEV */
