/**
 ******************************************************************************
 * @file    dsp_design_elliptic.c
 * @brief   طراحی فیلتر Elliptic (Cauer) — LP/HP/BP/BS
 *
 * الگوریتم بر اساس scipy.signal._filter_design.ellipap (مرجع اعتبارسنجی) و
 * یادداشت‌های Orfanidis ("Lecture Notes on Elliptic Filter Design", ECE-521):
 *
 *   1) εp = sqrt(10^(rp/10) − 1) ،  k1² = εp² / (10^(rs/10) − 1)
 *   2) m = حل معادله‌ی درجه‌ی بیضوی (روش نام‌ها): n·K(m)/K'(m) = K1/K1'
 *   3) صفرها: z_j = j / (sqrt(m)·sn(j·K(m)/n , m))
 *   4) قطب‌ها: v0 = K(m)·sc⁻¹(1/εp , k1²)/(n·K(k1²)) و
 *      p_j = −(cn·dn·sn(v0)·cn(v0) + j·sn·dn(v0)) / (1 − dn²·sn²(v0))
 *      که sn/cn/dn با مدول m و آرگومان v0 = ... (جزئیات کامل در کد)
 *   5) گین: مرتبه‌ی زوج → k = k/sqrt(1+εp²)
 *
 * مزایا: کمترین مرتبه برای مشخصات رipple/attenuation معین.
 * معایب: پیچیده‌ترین محاسبه (توابع بیضوی ژاکوبی)؛ در M7 فقط Design-Time.
 ******************************************************************************
 */

#include "dsp_config.h"

#if DSP_ENABLE_ELLIPTIC

#include "dsp_design.h"
#include "dsp_design_int.h"
#include "dsp_math.h"

#define DSP_ELLIP_EPS 2.220446049250313e-16

dsp_err_t dsp_design_elliptic(const dsp_design_params_t *cfg)
{
    int n, j;
    double eps_sq, eps, ck1_sq;
    double K1, m, capk;
    double r, v0;
    double sv0, cv0, dv0;
    dsp_cx_t p[DSP_MAX_DESIGN_ORDER];
    dsp_cx_t z[DSP_MAX_DESIGN_ORDER];
    int nz = 0, np = 0;
    double k;

    if (cfg == NULL) return DSP_ERR_NULL_PTR;
    n = cfg->order;
    if (n < 1 || n > DSP_MAX_DESIGN_ORDER) return DSP_ERR_INVALID_PARAMETER;
    if (cfg->ripple_db <= 0.0 || cfg->stop_db <= 0.0) return DSP_ERR_INVALID_PARAMETER;

    if (n == 1) {
        double pp = -sqrt(1.0 / expm1(log(10.0) * 0.1 * cfg->ripple_db));
        p[0] = dsp_cx_make(pp, 0.0);
        return dsp_design_run(cfg, p, 1, NULL, 0, -pp, 1.0);
    }

    eps_sq = expm1(log(10.0) * 0.1 * cfg->ripple_db);
    eps    = sqrt(eps_sq);
    ck1_sq = eps_sq / expm1(log(10.0) * 0.1 * cfg->stop_db);
    if (ck1_sq <= 0.0 || ck1_sq >= 1.0) return DSP_ERR_INVALID_PARAMETER;

    K1  = dsp_ellipk(ck1_sq);
    m   = dsp_ellipdeg(n, ck1_sq);
    capk = dsp_ellipk(m);

    /* صفرها:  z = j/(sqrt(m)·sn(j·capk/n , m))  برای j فردِ (1 یا 0 شروع) */
    {
        int start = (n & 1) ? 0 : 1;
        for (j = start; j < n; j += 2) {
            double sn, cn, dn;
            dsp_ellipj((double)j * capk / (double)n, m, &sn, &cn, &dn);
            if (fabs(sn) > DSP_ELLIP_EPS) {
                z[nz++] = dsp_cx_make(0.0, 1.0 / (sqrt(m) * sn));
            }
        }
    }
    /* مزدوج صفرها */
    {
        int n0 = nz;
        for (j = 0; j < n0; j++) {
            z[nz++] = dsp_cx_conj(z[j]);
        }
    }

    /* v0 و قطب‌ها */
    r   = dsp_arc_jac_sc1(1.0 / eps, ck1_sq);
    v0  = capk * r / ((double)n * K1);
    dsp_ellipj(v0, 1.0 - m, &sv0, &cv0, &dv0);

    {
        int start = (n & 1) ? 0 : 1;
        for (j = start; j < n; j += 2) {
            double sn, cn, dn;
            dsp_cx_t num, den;
            dsp_ellipj((double)j * capk / (double)n, m, &sn, &cn, &dn);
            /* p = -(c·d·sv0·cv0 + j·s·dv0) / (1 − (d·sv0)²) */
            num = dsp_cx_make(-cn * dn * sv0 * cv0, -sn * dv0);
            den = dsp_cx_make(1.0 - (dn * sv0) * (dn * sv0), 0.0);
            p[np++] = dsp_cx_div(num, den);
        }
    }

    if (n & 1) {
        /* قطب‌های مختلط را مزدوج اضافه کن (قطب حقیقی فقط یک‌بار) */
        double norm = 0.0;
        int i;
        int n0 = np;
        for (i = 0; i < n0; i++) norm += p[i].re * p[i].re + p[i].im * p[i].im;
        norm = sqrt(norm);
        for (i = 0; i < n0; i++) {
            if (fabs(p[i].im) > DSP_ELLIP_EPS * norm) {
                p[np++] = dsp_cx_conj(p[i]);
            }
        }
    } else {
        int n0 = np;
        int i;
        for (i = 0; i < n0; i++) p[np++] = dsp_cx_conj(p[i]);
    }

    k = dsp_prod_neg(p, np) / dsp_prod_neg(z, nz);
    if ((n & 1) == 0) k /= sqrt(1.0 + eps_sq);

    return dsp_design_run(cfg, p, np, z, nz, k,
                          (n & 1) ? 1.0 : pow(10.0, -cfg->ripple_db / 20.0));
}
#endif /* DSP_ENABLE_ELLIPTIC */
