#include "dsp_config.h"

#if DSP_ENABLE_BUTTERWORTH || DSP_ENABLE_CHEBYSHEV || DSP_ENABLE_ELLIPTIC || DSP_ENABLE_BESSEL

/**
 ******************************************************************************
 * @file    dsp_math.c
 * @brief   پیاده‌سازی موتور ریاضی Design-Time (رجوع کنید به dsp_math.h)
 *
 * الگوریتم‌ها بر اساس مراجع:
 *   - Orfanidis, "Lecture Notes on Elliptic Filter Design" (ECE-521, Rutgers)
 *   - scipy.signal._filter_design (مرجع اعتبارسنجی)
 *   - Abramowitz & Stegun (توابع بیضوی)
 ******************************************************************************
 */

#include "dsp_math.h"

/* ===========================================================================
 * عملیات مختلط
 * =========================================================================== */
dsp_cx_t dsp_cx_div(dsp_cx_t a, dsp_cx_t b)
{
    double d = b.re * b.re + b.im * b.im;
    return dsp_cx_make((a.re * b.re + a.im * b.im) / d,
                       (a.im * b.re - a.re * b.im) / d);
}

dsp_cx_t dsp_cx_sqrt(dsp_cx_t a)
{
    /* ریشه‌ی اصلی (principal): Re >= 0؛ برای اعداد منفی حقیقی شاخه‌ی +im
       (همان قرارداد scipy/numpy) */
    double t;
    if (a.re == 0.0 && a.im == 0.0) {
        return dsp_cx_make(0.0, 0.0);
    }
    t = hypot(a.re, a.im);
    return dsp_cx_make(sqrt((t + a.re) * 0.5),
                       (a.im >= 0.0) ? sqrt((t - a.re) * 0.5) : -sqrt((t - a.re) * 0.5));
}

static dsp_cx_t dsp_cx_log(dsp_cx_t a)
{
    return dsp_cx_make(log(hypot(a.re, a.im)), atan2(a.im, a.re));
}

static dsp_cx_t dsp_cx_asin(dsp_cx_t a)
{
    /* asin(z) = -i * log(i*z + sqrt(1 - z^2)) */
    dsp_cx_t i = dsp_cx_make(0.0, 1.0);
    dsp_cx_t z2 = dsp_cx_mul(a, a);
    dsp_cx_t s = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_make(1.0, 0.0), z2));
    dsp_cx_t t = dsp_cx_add(dsp_cx_mul(i, a), s);
    dsp_cx_t l = dsp_cx_log(t);
    return dsp_cx_make(l.im, -l.re);
}

dsp_cx_t dsp_cx_exp(dsp_cx_t a)
{
    double e = exp(a.re);
    return dsp_cx_make(e * cos(a.im), e * sin(a.im));
}

/* ===========================================================================
 * K(m) — انتگرال بیضوی کامل نوع اول با روش AGM
 *   K(m) = pi / (2 * AGM(1, sqrt(1-m)))
 * =========================================================================== */
double dsp_ellipk(double m)
{
    double a, b, c = 1.0;
    if (m < 0.0) m = 0.0;
    if (m >= 1.0) m = 1.0 - 1e-15;
    if (m <= 0.0) return DSP_PI / 2.0;

    a = 1.0;
    b = sqrt(1.0 - m);
    for (int i = 0; i < 64; i++) {
        double an = (a + b) * 0.5;
        double bn = sqrt(a * b);
        double cn = (a - b) * 0.5;
        a = an; b = bn;
        if (cn == 0.0 || fabs(cn) <= fabs(c) * 1e-17) break;
        c = cn;
    }
    return DSP_PI / (2.0 * a);
}

double dsp_ellipkm1(double m)
{
    return dsp_ellipk(1.0 - m);
}

/* ===========================================================================
 * توابع بیضوی ژاکوبی sn/cn/dn از طریق نام q و سری‌های تتا:
 *   v = u*pi/(2K)
 *   sn(u) = th3(0)*th1(v) / (th2(0)*th4(v))
 *   cn(u) = th4(0)*th2(v) / (th2(0)*th4(v))
 *   dn(u) = th4(0)*th3(v) / (th3(0)*th4(v))
 *
 * سری‌ها (برای دقت، با pow مستقیم — فقط در Design-Time استفاده می‌شود):
 *   th1(v) = 2*sum (-1)^{n+1} q^{(n-1/2)^2} sin((2n-1)v)
 *   th2(v) = 2*sum q^{(n-1/2)^2} cos((2n-1)v)
 *   th3(v) = 1 + 2*sum q^{n^2} cos(2nv)
 *   th4(v) = 1 + 2*sum (-1)^n q^{n^2} cos(2nv)
 * =========================================================================== */
void dsp_ellipj(double u, double m, double *sn, double *cn, double *dn)
{
    double K, q, v;
    double th2_0, th3_0, th4_0;
    double th1_v = 0.0, th2_v = 0.0, th3_v = 0.0, th4_v = 0.0;
    int n;

    if (m <= 0.0) { *sn = sin(u); *cn = cos(u); *dn = 1.0; return; }
    if (m >= 1.0) { double t = tanh(u); *sn = t; *cn = 1.0 / cosh(u); *dn = *cn; return; }

    K = dsp_ellipk(m);
    q = exp(-DSP_PI * dsp_ellipkm1(m) / K);
    if (q >= 1.0) q = 1.0 - 1e-15;
    v = u * DSP_PI / (2.0 * K);

    /* تتاها در v=0 */
    {
        double s1 = 0.0, s2 = 0.0, s3 = 0.0;
        for (n = 1; n <= 64; n++) {
            double qn2 = pow(q, (double)(n * n));
            s1 += qn2;
            s2 += ((n & 1) ? -qn2 : qn2);
            s3 += pow(q, (double)(n * (n - 1)));   /* q^{n(n-1)} : جمله‌ی اول = 1 */
            if (qn2 < 1e-300) break;
        }
        th2_0 = 2.0 * pow(q, 0.25) * s3;
        th3_0 = 1.0 + 2.0 * s1;
        th4_0 = 1.0 + 2.0 * s2;
    }

    /* تتاها در v */
    {
        double t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0;
        for (n = 1; n <= 64; n++) {
            double qn1 = pow(q, (n - 0.5) * (n - 0.5));
            double qn2 = pow(q, (double)(n * n));
            double cv = cos((2.0 * n - 1.0) * v);
            double sv = sin((2.0 * n - 1.0) * v);
            double c2 = cos(2.0 * n * v);
            t1 += ((n & 1) ? 1.0 : -1.0) * qn1 * sv;
            t2 += qn1 * cv;
            t3 += qn2 * c2;
            t4 += ((n & 1) ? -qn2 : qn2) * c2;
            if (qn2 < 1e-300) break;
        }
        th1_v = 2.0 * t1;
        th2_v = 2.0 * t2;
        th3_v = 1.0 + 2.0 * t3;
        th4_v = 1.0 + 2.0 * t4;
    }

    {
        double den = th2_0 * th4_v;
        if (den == 0.0) den = 1e-300;
        *sn = (th3_0 * th1_v) / den;
        *cn = (th4_0 * th2_v) / den;
        *dn = (th4_0 * th3_v) / (th3_0 * th4_v);
    }
}

/* ===========================================================================
 * معکوس sn — روش تبدیل Landen نزولی (Orfanidis Eq. 56)
 * حل z به‌طوری که sn(z, m) = w   (w مختلط، m در [0,1])
 * =========================================================================== */
dsp_cx_t dsp_arc_jac_sn(dsp_cx_t w, double m)
{
    double k = sqrt(m);
    double ks[32];
    dsp_cx_t wns[32];
    double Kfac = 1.0;
    int niter = 0;
    int i;

    if (k > 1.0) return dsp_cx_make(0.0, 0.0);
    if (k == 1.0) {
        return dsp_cx_asin(w);
    }

    ks[0] = k;
    while (ks[niter] > 0.0) {
        double k_ = ks[niter];
        double k_p = sqrt((1.0 - k_) * (1.0 + k_));
        double kn = (1.0 - k_p) / (1.0 + k_p);
        ks[niter + 1] = kn;
        niter++;
        if (niter > 24) break;
    }
    for (i = 1; i <= niter; i++) {
        Kfac *= (1.0 + ks[i]);
    }
    Kfac *= DSP_PI / 2.0;

    wns[0] = w;
    for (i = 0; i < niter; i++) {
        double kn = ks[i];
        double knext = ks[i + 1];
        dsp_cx_t wn = wns[i];
        /* sqrt(1 - (kn*wn)^2) */
        dsp_cx_t t = dsp_cx_mul(dsp_cx_scale(wn, kn), dsp_cx_scale(wn, kn));
        dsp_cx_t c = dsp_cx_sqrt(dsp_cx_sub(dsp_cx_make(1.0, 0.0), t));
        dsp_cx_t den = dsp_cx_mul(dsp_cx_make(1.0 + knext, 0.0),
                                  dsp_cx_add(dsp_cx_make(1.0, 0.0), c));
        wns[i + 1] = dsp_cx_div(dsp_cx_scale(wn, 2.0), den);
    }

    {
        dsp_cx_t u = dsp_cx_scale(dsp_cx_asin(wns[niter]), 2.0 / DSP_PI);
        return dsp_cx_scale(u, Kfac);
    }
}

/* ===========================================================================
 * معکوس sc با مدول مکمل:
 *   w = sc(z, 1-m) = -i*sn(i*z, 1-m)   =>   z = -i * asn(i*w, 1-m)
 * آرگومان ورودی w حقیقی است؛ خروجی: بخش موهومی (بخش حقیقی ~0)
 * =========================================================================== */
double dsp_arc_jac_sc1(double w, double m)
{
    /* مطابق scipy:  _arc_jac_sc1(w, m) = imag(_arc_jac_sn(1j*w, m)) */
    dsp_cx_t zc = dsp_arc_jac_sn(dsp_cx_make(0.0, w), m);
    return zc.im;
}

/* ===========================================================================
 * حل معادله‌ی درجه‌ی بیضوی با روش نام‌ها (Orfanidis Eq. 49)
 *   n * K(m)/K'(m) = K1(m1)/K1'(m1)  ->  m
 * =========================================================================== */
double dsp_ellipdeg(int n, double m1)
{
    double K1 = dsp_ellipk(m1);
    double K1p = dsp_ellipkm1(m1);
    double q1 = exp(-DSP_PI * K1p / K1);
    double q = pow(q1, 1.0 / n);
    double num = 0.0, den = 0.0;
    int i;

    for (i = 0; i <= 7; i++) {
        num += pow(q, (double)(i * (i + 1)));
    }
    den = 1.0;
    for (i = 1; i <= 8; i++) {
        den += 2.0 * pow(q, (double)(i * i));
    }
    return 16.0 * q * pow(num / den, 4.0);
}

/* ===========================================================================
 * ریشه‌یابی چندجمله‌ای — روش Durand-Kerner (Weierstrass)
 * p: ضرایب (بالاترین توان اول)، n: درجه، roots: خروجی به طول n
 * =========================================================================== */
dsp_err_t dsp_poly_roots(const double *p, int n, dsp_cx_t *roots)
{
    dsp_cx_t *z;
    int i, iter;
    const double eps = 1e-14;

    if (p == NULL || roots == NULL || n < 1) return DSP_ERR_INVALID_PARAMETER;
    if (p[0] == 0.0) return DSP_ERR_INVALID_PARAMETER;

    z = roots;
    {
        dsp_cx_t base = dsp_cx_make(0.4, 0.9);
        dsp_cx_t cur = dsp_cx_make(1.0, 0.0);
        for (i = 0; i < n; i++) {
            z[i] = cur;
            cur = dsp_cx_mul(cur, base);
        }
    }

    for (iter = 0; iter < 400; iter++) {
        double maxdelta = 0.0;
        for (i = 0; i < n; i++) {
            dsp_cx_t num = dsp_cx_make(p[0], 0.0);
            dsp_cx_t den = dsp_cx_make(1.0, 0.0);
            int j;
            for (j = 1; j <= n; j++) {
                num = dsp_cx_add(dsp_cx_mul(num, z[i]), dsp_cx_make(p[j], 0.0));
            }
            for (j = 0; j < n; j++) {
                if (j != i) den = dsp_cx_mul(den, dsp_cx_sub(z[i], z[j]));
            }
            if (dsp_cx_abs(den) < 1e-300) continue;
            {
                dsp_cx_t delta = dsp_cx_div(num, den);
                z[i] = dsp_cx_sub(z[i], delta);
                if (dsp_cx_abs(delta) > maxdelta) maxdelta = dsp_cx_abs(delta);
            }
        }
        if (maxdelta < eps) break;
    }

    /* حذف نویز موهومی ریشه‌های حقیقی */
    {
        double maxc = 0.0;
        for (i = 0; i < n; i++) {
            if (dsp_cx_abs(z[i]) > maxc) maxc = dsp_cx_abs(z[i]);
        }
        for (i = 0; i < n; i++) {
            if (fabs(z[i].im) < 1e-12 * (1.0 + maxc)) z[i].im = 0.0;
        }
    }
    return DSP_OK;
}

/* ===========================================================================
 * ضرایب چندجمله‌ای بسل  y_N(s) = sum_{k=0..N} c_k s^k
 *   c_k = (N+k)! / ((N-k)! k! 2^k)
 * رابطه‌ی بازگشتی:  c_{k+1} = c_k * (N-k)(N+k+1) / (2(k+1))
 * =========================================================================== */
void dsp_bessel_poly_coeffs(int order, double *coeffs)
{
    int k;
    double c = 1.0;   /* c_0 = 1 */
    coeffs[0] = c;
    for (k = 0; k < order; k++) {
        c = c * (double)((order - k) * (order + k + 1)) / (2.0 * (double)(k + 1));
        coeffs[k + 1] = c;
    }
}

#endif /* any design feature */
