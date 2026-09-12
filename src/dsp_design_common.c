#include "dsp_config.h"

#if DSP_ENABLE_BUTTERWORTH || DSP_ENABLE_CHEBYSHEV || DSP_ENABLE_ELLIPTIC || DSP_ENABLE_BESSEL
/**
 ******************************************************************************
 * @file    dsp_design_common.c  (internal)
 * @brief   هسته‌ی مشترک موتور طراحی فیلتر
 *
 * قراردادها (مطابق scipy برای اعتبارسنجی متقابل):
 *   - Bilinear با T=2:  s = (z-1)/(z+1) ،  z_d = (1+s)/(1-s)
 *   - پیش‌وارپ: Ω = tan(π f / fs)
 *   - BP/BS با پهنای‌باند هندسی: wo = sqrt(Ωl·Ωh) ، bw = Ωh − Ωl
 ******************************************************************************
 */

#include "dsp_design.h"
#include "dsp_design_int.h"
#include "dsp_math.h"
#include <string.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * ابزار مشترک: حاصل‌ضرب (−v_i) برای محاسبه‌ی گین پروتوتایپ
 * ------------------------------------------------------------------------- */
double dsp_prod_neg(const dsp_cx_t *v, int n)
{
    dsp_cx_t p = dsp_cx_make(1.0, 0.0);
    int i;
    for (i = 0; i < n; i++) {
        p = dsp_cx_mul(p, dsp_cx_make(-v[i].re, -v[i].im));
    }
    return p.re;
}

/* ---------------------------------------------------------------------------
 * تبدیل‌های فرکانسی آنالوگ (ZPK)
 * ------------------------------------------------------------------------- */

/* LP → LP:  s ← s/wo */
void dsp_design_lp2lp(dsp_zpk_t *zpk, double wo)
{
    int i;
    int deg = zpk->np - zpk->nz;
    for (i = 0; i < zpk->nz; i++) zpk->z[i] = dsp_cx_scale(zpk->z[i], wo);
    for (i = 0; i < zpk->np; i++) zpk->p[i] = dsp_cx_scale(zpk->p[i], wo);
    zpk->k *= pow(wo, deg);
}

/* LP → HP:  s ← wo/s  (صفرهای جدید در مبدأ؛ گین بر اساس نسبت صفرها/قطب‌ها) */
void dsp_design_lp2hp(dsp_zpk_t *zpk, double wo)
{
    int i;
    dsp_cx_t zn[DSP_ZPK_MAX], pn[DSP_ZPK_MAX];
    double kn;

    /* محصول (-z)/(-p) برای گین */
    {
        dsp_cx_t pz = dsp_cx_make(1.0, 0.0), pp = dsp_cx_make(1.0, 0.0);
        for (i = 0; i < zpk->nz; i++) pz = dsp_cx_mul(pz, dsp_cx_make(-zpk->z[i].re, -zpk->z[i].im));
        for (i = 0; i < zpk->np; i++) pp = dsp_cx_mul(pp, dsp_cx_make(-zpk->p[i].re, -zpk->p[i].im));
        kn = dsp_cx_div(pz, pp).re;
    }
    for (i = 0; i < zpk->np; i++) {
        pn[i] = dsp_cx_div(dsp_cx_make(wo, 0.0), zpk->p[i]);
    }
    for (i = 0; i < zpk->nz; i++) {
        if (dsp_cx_abs(zpk->z[i]) > 0.0) {
            zn[i] = dsp_cx_div(dsp_cx_make(wo, 0.0), zpk->z[i]);
        } else {
            zn[i] = dsp_cx_make(0.0, 0.0);
        }
    }
    /* صفرهای درجه‌ای به مبدأ منتقل می‌شوند */
    for (i = zpk->nz; i < zpk->np; i++) zn[i] = dsp_cx_make(0.0, 0.0);

    zpk->nz = zpk->np;
    for (i = 0; i < zpk->np; i++) { zpk->z[i] = zn[i]; zpk->p[i] = pn[i]; }
    zpk->k *= kn;
}

/* LP → BP:  s ← (s² + wo²)/(s·bw)  — هر قطب/صفر به یک جفت تبدیل می‌شود */
void dsp_design_lp2bp(dsp_zpk_t *zpk, double wo, double bw)
{
    int i, j, deg = zpk->np - zpk->nz;
    dsp_cx_t zn[DSP_ZPK_MAX * 2], pn[DSP_ZPK_MAX * 2];
    int nz2 = 0, np2 = 0;
    dsp_cx_t hbw = dsp_cx_make(bw / 2.0, 0.0);

    for (i = 0; i < zpk->np; i++) {
        dsp_cx_t pl = dsp_cx_mul(zpk->p[i], hbw);
        dsp_cx_t sq = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_mul(pl, pl),
                                             dsp_cx_make(wo * wo, 0.0)));
        pn[np2++] = dsp_cx_add(pl, sq);
        pn[np2++] = dsp_cx_sub(pl, sq);
    }
    for (i = 0; i < zpk->nz; i++) {
        dsp_cx_t zl = dsp_cx_mul(zpk->z[i], hbw);
        dsp_cx_t sq = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_mul(zl, zl),
                                             dsp_cx_make(wo * wo, 0.0)));
        zn[nz2++] = dsp_cx_add(zl, sq);
        zn[nz2++] = dsp_cx_sub(zl, sq);
    }
    /* صفرهای درجه‌ای در مبدأ قرار می‌گیرند */
    for (j = 0; j < deg; j++) zn[nz2++] = dsp_cx_make(0.0, 0.0);

    zpk->nz = nz2; zpk->np = np2;
    for (i = 0; i < np2; i++) zpk->p[i] = pn[i];
    for (i = 0; i < nz2; i++) zpk->z[i] = zn[i];
    zpk->k *= pow(bw, deg);
}

/* LP → BS:  s ← s·bw/(s² + wo²) */
void dsp_design_lp2bs(dsp_zpk_t *zpk, double wo, double bw)
{
    int i, j, deg = zpk->np - zpk->nz;
    dsp_cx_t zn[DSP_ZPK_MAX * 2], pn[DSP_ZPK_MAX * 2];
    int nz2 = 0, np2 = 0;
    dsp_cx_t hbw = dsp_cx_make(bw / 2.0, 0.0);
    dsp_cx_t pz = dsp_cx_make(1.0, 0.0), pp = dsp_cx_make(1.0, 0.0);

    for (i = 0; i < zpk->nz; i++) pz = dsp_cx_mul(pz, dsp_cx_make(-zpk->z[i].re, -zpk->z[i].im));
    for (i = 0; i < zpk->np; i++) pp = dsp_cx_mul(pp, dsp_cx_make(-zpk->p[i].re, -zpk->p[i].im));

    /* صفرهای HP معادل: (bw/2)/z و (bw/2)/p */
    for (i = 0; i < zpk->nz; i++) {
        dsp_cx_t zh = dsp_cx_div(hbw, zpk->z[i]);
        dsp_cx_t sq = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_mul(zh, zh),
                                             dsp_cx_make(wo * wo, 0.0)));
        zn[nz2++] = dsp_cx_add(zh, sq);
        zn[nz2++] = dsp_cx_sub(zh, sq);
    }
    for (i = 0; i < zpk->np; i++) {
        dsp_cx_t ph = dsp_cx_div(hbw, zpk->p[i]);
        dsp_cx_t sq = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_mul(ph, ph),
                                             dsp_cx_make(wo * wo, 0.0)));
        pn[np2++] = dsp_cx_add(ph, sq);
        pn[np2++] = dsp_cx_sub(ph, sq);
    }
    /* صفرهای مبدأ به مرکز باند (j·wo) منتقل می‌شوند */
    for (j = 0; j < deg; j++) {
        zn[nz2++] = dsp_cx_make(0.0, wo);
        zn[nz2++] = dsp_cx_make(0.0, -wo);
    }

    zpk->nz = nz2; zpk->np = np2;
    for (i = 0; i < np2; i++) zpk->p[i] = pn[i];
    for (i = 0; i < nz2; i++) zpk->z[i] = zn[i];
    zpk->k *= dsp_cx_div(pz, pp).re;
}

/* ---------------------------------------------------------------------------
 * Bilinear — T=2:  z_d = (1+s)/(1-s) ،  k_d = k·Π(1−z_i)/Π(1−p_i)
 * صفرهای (np − nz) در z = −1 اضافه می‌شوند (اثر (z+1)^{np−nz}).
 * ------------------------------------------------------------------------- */
void dsp_design_bilinear(dsp_zpk_t *zpk)
{
    int i;
    dsp_cx_t zn[DSP_ZPK_MAX * 2], pn[DSP_ZPK_MAX * 2];
    int nz2 = 0;
    dsp_cx_t kz = dsp_cx_make(1.0, 0.0), kp = dsp_cx_make(1.0, 0.0);

    for (i = 0; i < zpk->nz; i++) {
        kz = dsp_cx_mul(kz, dsp_cx_sub(dsp_cx_make(1.0, 0.0), zpk->z[i]));
    }
    for (i = 0; i < zpk->np; i++) {
        kp = dsp_cx_mul(kp, dsp_cx_sub(dsp_cx_make(1.0, 0.0), zpk->p[i]));
    }

    for (i = 0; i < zpk->nz; i++) {
        zn[nz2++] = dsp_cx_div(dsp_cx_add(dsp_cx_make(1.0, 0.0), zpk->z[i]),
                              dsp_cx_sub(dsp_cx_make(1.0, 0.0), zpk->z[i]));
    }
    for (i = 0; i < zpk->np; i++) {
        pn[i] = dsp_cx_div(dsp_cx_add(dsp_cx_make(1.0, 0.0), zpk->p[i]),
                          dsp_cx_sub(dsp_cx_make(1.0, 0.0), zpk->p[i]));
    }
    /* صفرهای اضافی در z = −1 */
    for (i = 0; i < zpk->np - zpk->nz; i++) {
        zn[nz2++] = dsp_cx_make(-1.0, 0.0);
    }

    zpk->nz = nz2;   /* np تغییر نمی‌کند */
    for (i = 0; i < zpk->nz; i++) zpk->z[i] = zn[i];
    for (i = 0; i < zpk->np; i++) zpk->p[i] = pn[i];
    zpk->k *= dsp_cx_div(kz, kp).re;
}

/* ---------------------------------------------------------------------------
 * ارزیابی H(e^{jw}) برای زنجیره‌ی SOS — مختلط
 * ------------------------------------------------------------------------- */
dsp_cx_t dsp_design_sos_eval(const dsp_f64_t *sos, uint16_t nSections, dsp_f64_t w)
{
    dsp_cx_t h = dsp_cx_make(1.0, 0.0);
    uint16_t s;
    dsp_cx_t z1 = dsp_cx_make(cos(-w), sin(-w));   /* e^{-jw} */
    dsp_cx_t z2 = dsp_cx_make(cos(-2.0 * w), sin(-2.0 * w));

    for (s = 0; s < nSections; s++) {
        const dsp_f64_t *c = sos + 5 * s;
        dsp_cx_t num, den, r;
        num = dsp_cx_add(dsp_cx_make(c[0], 0.0),
              dsp_cx_add(dsp_cx_mul(z1, dsp_cx_make(c[1], 0.0)),
                         dsp_cx_mul(z2, dsp_cx_make(c[2], 0.0))));
        den = dsp_cx_add(dsp_cx_make(1.0, 0.0),
              dsp_cx_add(dsp_cx_mul(z1, dsp_cx_make(c[3], 0.0)),
                         dsp_cx_mul(z2, dsp_cx_make(c[4], 0.0))));
        r = dsp_cx_div(num, den);
        h = dsp_cx_mul(h, r);
    }
    return h;
}

/* ---------------------------------------------------------------------------
 * ساخت SOS از ZPK دیجیتال + نرمال‌سازی گین در فرکانس مرجع
 * ------------------------------------------------------------------------- */
static int dsp_design_cmp_angle(const void *a, const void *b)
{
    const dsp_cx_t *ca = (const dsp_cx_t *)a;
    const dsp_cx_t *cb = (const dsp_cx_t *)b;
    double aa = (dsp_cx_abs(*ca) < 1e-15) ? 0.0 : atan2(ca->im, ca->re);
    double ab = (dsp_cx_abs(*cb) < 1e-15) ? 0.0 : atan2(cb->im, cb->re);
    if (aa < 0) aa += DSP_TWO_PI;
    if (ab < 0) ab += DSP_TWO_PI;
    return (aa < ab) ? -1 : (aa > ab) ? 1 : 0;
}

dsp_err_t dsp_design_zpk2sos(const dsp_design_params_t *cfg, const dsp_zpk_t *zpk,
                               double target_gain)
{
    dsp_cx_t p[DSP_ZPK_MAX], z[DSP_ZPK_MAX];
    int np, nz, i;
    int nsec, maxsec = DSP_MAX_SOS_STAGES;
    int used_z = 0;
    double gain_total;

    if (cfg == NULL || cfg->pSOS == NULL || cfg->pNumSections == NULL) {
        return DSP_ERR_NULL_PTR;
    }
    np = zpk->np;
    nz = zpk->nz;
    nsec = (np + 1) / 2;
    if (nsec > maxsec) return DSP_ERR_INVALID_PARAMETER;

    memcpy(p, zpk->p, sizeof(dsp_cx_t) * (size_t)np);
    memcpy(z, zpk->z, sizeof(dsp_cx_t) * (size_t)nz);

    /* مرتب‌سازی بر اساس زاویه برای جفت‌کردن مزدوج‌ها و نزدیک‌ترین صفر */
    qsort(p, (size_t)np, sizeof(dsp_cx_t), dsp_design_cmp_angle);
    qsort(z, (size_t)nz, sizeof(dsp_cx_t), dsp_design_cmp_angle);

    gain_total = zpk->k;

    /* گروه‌بندی صفرها: جفت‌های مزدوج (نماینده با im>0) و صفرهای حقیقی */
    {
        dsp_cx_t zc[DSP_ZPK_MAX];
        double  zr[DSP_ZPK_MAX];
        int nzc = 0, nzr = 0, ic = 0, ir = 0;
        dsp_bool_t used[DSP_ZPK_MAX];
        int j;
        memset(used, 0, sizeof(used));

        for (j = 0; j < nz; j++) {
            if (fabs(z[j].im) > 1e-12 && !used[j]) {
                /* پیدا کردن مزدوج */
                int k, found = -1;
                for (k = j + 1; k < nz; k++) {
                    if (!used[k] && fabs(z[k].re - z[j].re) < 1e-9 &&
                        fabs(z[k].im + z[j].im) < 1e-9) { found = k; break; }
                }
                if (found >= 0) {
                    zc[nzc++] = z[j];   /* نماینده با im>0 (مرتب‌سازی بعدی) */
                    used[j] = DSP_TRUE; used[found] = DSP_TRUE;
                } else {
                    zr[nzr++] = z[j].re;
                    used[j] = DSP_TRUE;
                }
            } else if (fabs(z[j].im) <= 1e-12) {
                zr[nzr++] = z[j].re;
                used[j] = DSP_TRUE;
            }
        }
        /* مرتب‌سازی نماینده‌های مختلط بر اساس زاویه و حقیقی‌ها بر اساس مقدار */
        qsort(zc, (size_t)nzc, sizeof(dsp_cx_t), dsp_design_cmp_angle);
        {
            int a, b;
            for (a = 0; a < nzr; a++) {
                for (b = a + 1; b < nzr; b++) {
                    if (zr[b] < zr[a]) { double t = zr[a]; zr[a] = zr[b]; zr[b] = t; }
                }
            }
        }

        for (i = 0; i < nsec; i++) {
            dsp_f64_t *c = cfg->pSOS + 5 * i;
            dsp_cx_t p1 = p[2 * i];
            int poleComplex = (fabs(p1.im) > 1e-12);

            /* مخرج */
            if (!poleComplex) {
                c[3] = -p1.re;
                c[4] = 0.0;
            } else {
                c[3] = -2.0 * p1.re;
                c[4] = p1.re * p1.re + p1.im * p1.im;
            }

            /* صورت: تخصیص صفرها به طبقات */
            if (poleComplex) {
                if (ic < nzc) {
                    c[0] = 1.0;
                    c[1] = -2.0 * zc[ic].re;
                    c[2] = zc[ic].re * zc[ic].re + zc[ic].im * zc[ic].im;
                    ic++;
                } else if (ir + 1 < nzr) {
                    c[0] = 1.0; c[1] = -(zr[ir] + zr[ir + 1]); c[2] = zr[ir] * zr[ir + 1];
                    ir += 2;
                } else if (ir < nzr) {
                    c[0] = 1.0; c[1] = -zr[ir]; c[2] = 0.0;
                    ir++;
                } else {
                    c[0] = 1.0; c[1] = 0.0; c[2] = 0.0;
                }
            } else {
                if (ir < nzr) {
                    c[0] = 1.0; c[1] = -zr[ir]; c[2] = 0.0;
                    ir++;
                } else if (ic < nzc) {
                    c[0] = 1.0;
                    c[1] = -2.0 * zc[ic].re;
                    c[2] = zc[ic].re * zc[ic].re + zc[ic].im * zc[ic].im;
                    ic++;
                } else {
                    c[0] = 1.0; c[1] = 0.0; c[2] = 0.0;
                }
            }
        }
        (void)used_z;
    }

    *cfg->pNumSections = (uint16_t)nsec;

    /* --- نرمال‌سازی گین در فرکانس مرجع --- */
    {
        dsp_f64_t wref;
        dsp_cx_t hv;
        double mag;

        switch (cfg->type) {
            case DSP_FILTER_LP:
            case DSP_FILTER_BS:
                wref = 0.0;               /* DC */
                break;
            case DSP_FILTER_HP:
                wref = DSP_PI;            /* Nyquist */
                break;
            default: {                    /* BP: مرکز باند */
                dsp_f64_t w1 = tan(DSP_PI * cfg->fc / cfg->fs);
                dsp_f64_t w2 = tan(DSP_PI * cfg->fc2 / cfg->fs);
                dsp_f64_t wo = sqrt(w1 * w2);
                wref = 2.0 * atan(wo);
                break;
            }
        }
        hv = dsp_design_sos_eval(cfg->pSOS, (uint16_t)nsec, wref);
        mag = dsp_cx_abs(hv);
        if (mag < 1e-300) mag = 1e-300;

        /* توزیع یکنواخت گین بین طبقات — target_gain برای Chebyshev I / Elliptic
           مرتبه‌ی زوج (گین DC = 10^(-rp/20)) وگرنه 1 */
        {
            double g = pow(target_gain / mag, 1.0 / (double)nsec);
            for (i = 0; i < nsec; i++) {
                cfg->pSOS[5 * i + 0] *= g;
                cfg->pSOS[5 * i + 1] *= g;
                cfg->pSOS[5 * i + 2] *= g;
            }
        }
        if (cfg->pGain != NULL) *cfg->pGain = gain_total;
    }
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * تبدیل‌های خروجی
 * ------------------------------------------------------------------------- */
void dsp_design_sos_f64_to_f32(const dsp_f64_t *src, uint16_t nSections, dsp_f32_t *dst)
{
    uint16_t i;
    for (i = 0; i < (uint16_t)(5 * nSections); i++) dst[i] = (dsp_f32_t)src[i];
}

static void dsp_design_quant_sos(const dsp_f64_t *src, uint16_t nSections,
                                 int32_t *dst, uint8_t *pShift, int bits)
{
    double maxa = 0.0;
    uint16_t i, n = (uint16_t)(5 * nSections);
    int s;
    double scale;

    for (i = 0; i < n; i++) {
        double a = fabs(src[i]);
        if (a > maxa) maxa = a;
    }
    if (maxa <= 0.0) maxa = 1.0;
    {
        int ce = (int)ceil(log(maxa) / log(2.0));
        s = -1 - ce;
    }
    if (s < -bits) s = -bits;
    scale = exp2((double)(bits - 1 + s));
    for (i = 0; i < n; i++) {
        double v = src[i] * scale;
        if (v >= 0.0) v += 0.5; else v -= 0.5;
        if (v > 2147483647.0) v = 2147483647.0;
        if (v < -2147483648.0) v = -2147483648.0;
        dst[i] = (int32_t)v;
    }
    *pShift = (uint8_t)((bits - 1) + s);
}

dsp_err_t dsp_design_sos_f64_to_q15(const dsp_f64_t *src, uint16_t nSections,
                                    dsp_q15_t *dst, uint8_t *pShift)
{
    int32_t tmp[DSP_MAX_SOS_STAGES * 5];
    uint16_t i;
    if (src == NULL || dst == NULL || pShift == NULL) return DSP_ERR_NULL_PTR;
    if (nSections == 0 || nSections > DSP_MAX_SOS_STAGES) return DSP_ERR_INVALID_PARAMETER;
    dsp_design_quant_sos(src, nSections, tmp, pShift, 16);
    for (i = 0; i < (uint16_t)(5 * nSections); i++) dst[i] = (dsp_q15_t)tmp[i];
    return DSP_OK;
}

dsp_err_t dsp_design_sos_f64_to_q31(const dsp_f64_t *src, uint16_t nSections,
                                    dsp_q31_t *dst, uint8_t *pShift)
{
    int32_t tmp[DSP_MAX_SOS_STAGES * 5];
    uint16_t i;
    if (src == NULL || dst == NULL || pShift == NULL) return DSP_ERR_NULL_PTR;
    if (nSections == 0 || nSections > DSP_MAX_SOS_STAGES) return DSP_ERR_INVALID_PARAMETER;
    dsp_design_quant_sos(src, nSections, tmp, pShift, 32);
    for (i = 0; i < (uint16_t)(5 * nSections); i++) dst[i] = (dsp_q31_t)tmp[i];
    return DSP_OK;
}

/* ---------------------------------------------------------------------------
 * پاسخ اندازه‌ی SOS — عمومی
 * ------------------------------------------------------------------------- */
dsp_f64_t dsp_design_sos_magnitude(const dsp_f64_t *sos, uint16_t nSections, dsp_f64_t w)
{
    return dsp_cx_abs(dsp_design_sos_eval(sos, nSections, w));
}

/* ---------------------------------------------------------------------------
 * تخمین مرتبه‌ی Butterworth
 * ------------------------------------------------------------------------- */
dsp_f64_t dsp_design_butterworth_estimate_order(dsp_f64_t fs, dsp_f64_t fpass,
                                                dsp_f64_t fstop, dsp_f64_t apass_db,
                                                dsp_f64_t astop_db)
{
    dsp_f64_t wp = tan(DSP_PI * fpass / fs);
    dsp_f64_t ws = tan(DSP_PI * fstop / fs);
    dsp_f64_t num = log10((pow(10.0, astop_db / 10.0) - 1.0) /
                          (pow(10.0, apass_db / 10.0) - 1.0));
    dsp_f64_t den = 2.0 * log10(ws / wp);
    dsp_f64_t n = num / den;
    return n;
}

/* ---------------------------------------------------------------------------
 * خط لوله‌ی کامل طراحی
 * ------------------------------------------------------------------------- */
dsp_err_t dsp_design_run(const dsp_design_params_t *cfg,
                         const dsp_cx_t *poles, int np,
                         const dsp_cx_t *zeros, int nz, double k,
                         double target_gain)
{
    dsp_zpk_t zpk;
    dsp_err_t err;
    dsp_f64_t w1, w2, wo, bw;

    if (cfg == NULL || poles == NULL || np < 1 || np > DSP_MAX_DESIGN_ORDER) {
        return DSP_ERR_INVALID_PARAMETER;
    }
    if (cfg->fs <= 0.0 || cfg->fc <= 0.0) return DSP_ERR_INVALID_PARAMETER;
    if (cfg->type == DSP_FILTER_BP || cfg->type == DSP_FILTER_BS) {
        if (cfg->fc2 <= cfg->fc || cfg->fc2 >= cfg->fs / 2.0) return DSP_ERR_INVALID_PARAMETER;
    } else {
        if (cfg->fc >= cfg->fs / 2.0) return DSP_ERR_INVALID_PARAMETER;
    }

    zpk.nz = nz;
    zpk.np = np;
    for (int i = 0; i < np; i++) zpk.p[i] = poles[i];
    for (int i = 0; i < nz; i++) zpk.z[i] = zeros[i];
    zpk.k = k;

    switch (cfg->type) {
        case DSP_FILTER_LP:
            dsp_design_lp2lp(&zpk, tan(DSP_PI * cfg->fc / cfg->fs));
            break;
        case DSP_FILTER_HP:
            dsp_design_lp2hp(&zpk, tan(DSP_PI * cfg->fc / cfg->fs));
            break;
        case DSP_FILTER_BP:
        case DSP_FILTER_BS:
            w1 = tan(DSP_PI * cfg->fc / cfg->fs);
            w2 = tan(DSP_PI * cfg->fc2 / cfg->fs);
            wo = sqrt(w1 * w2);
            bw = w2 - w1;
            if (cfg->type == DSP_FILTER_BP) dsp_design_lp2bp(&zpk, wo, bw);
            else                           dsp_design_lp2bs(&zpk, wo, bw);
            break;
        default:
            return DSP_ERR_INVALID_PARAMETER;
    }

    dsp_design_bilinear(&zpk);
    err = dsp_design_zpk2sos(cfg, &zpk, target_gain);
    return err;
}

#endif /* any design feature */
