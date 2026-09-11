#include "dsp_config.h"

#if DSP_ENABLE_BIQUAD
/**
 ******************************************************************************
 * @file    dsp_biquad_rbj.c  (internal)
 * @brief   پیاده‌سازی فرمول‌های RBJ و کمی‌سازی Fixed-Point برای Biquad
 ******************************************************************************
 */

#include "dsp_biquad_int.h"
#include "dsp_math.h"

void dsp_biquad_rbj_f64(uint8_t type, uint32_t fs,
                        double fc, double q, double gain_db, double c[5])
{
    double A, w0, alpha, cosw0, sqA;
    double b0, b1, b2, a0, a1, a2;
    double f = (double)fc / (double)fs;

    if (f <= 0.0 || f >= 0.5 || q <= 0.0) {
        c[0]=1; c[1]=0; c[2]=0; c[3]=0; c[4]=0;   /* بای‌پس ایمن */
        return;
    }

    w0    = DSP_TWO_PI * f;
    cosw0 = cos(w0);
    alpha = sin(w0) / (2.0 * q);
    A     = pow(10.0, gain_db / 40.0);
    sqA   = 2.0 * sqrt(A) * alpha;

    switch (type) {
        case DSP_BIQUAD_LPF:
            b0 = (1.0 - cosw0) / 2.0;  b1 = 1.0 - cosw0;  b2 = b0;
            a0 = 1.0 + alpha;          a1 = -2.0 * cosw0; a2 = 1.0 - alpha;
            break;
        case DSP_BIQUAD_HPF:
            b0 = (1.0 + cosw0) / 2.0;  b1 = -(1.0 + cosw0); b2 = b0;
            a0 = 1.0 + alpha;          a1 = -2.0 * cosw0;   a2 = 1.0 - alpha;
            break;
        case DSP_BIQUAD_BPF:   /* پیک 0 dB در مرکز */
            b0 = alpha; b1 = 0.0; b2 = -alpha;
            a0 = 1.0 + alpha; a1 = -2.0 * cosw0; a2 = 1.0 - alpha;
            break;
        case DSP_BIQUAD_BSF:
        case DSP_BIQUAD_NOTCH:
            b0 = 1.0; b1 = -2.0 * cosw0; b2 = 1.0;
            a0 = 1.0 + alpha; a1 = -2.0 * cosw0; a2 = 1.0 - alpha;
            break;
        case DSP_BIQUAD_PEAK:
            b0 = 1.0 + alpha * A; b1 = -2.0 * cosw0; b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A; a1 = -2.0 * cosw0; a2 = 1.0 - alpha / A;
            break;
        case DSP_BIQUAD_LSHELF:
            b0 = A * ((A + 1.0) - (A - 1.0) * cosw0 + sqA);
            b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
            b2 = A * ((A + 1.0) - (A - 1.0) * cosw0 - sqA);
            a0 = (A + 1.0) + (A - 1.0) * cosw0 + sqA;
            a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
            a2 = (A + 1.0) + (A - 1.0) * cosw0 - sqA;
            break;
        case DSP_BIQUAD_HSHELF:
            b0 = A * ((A + 1.0) + (A - 1.0) * cosw0 + sqA);
            b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
            b2 = A * ((A + 1.0) + (A - 1.0) * cosw0 - sqA);
            a0 = (A + 1.0) - (A - 1.0) * cosw0 + sqA;
            a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
            a2 = (A + 1.0) - (A - 1.0) * cosw0 - sqA;
            break;
        default:
            b0 = 1.0; b1 = 0.0; b2 = 0.0; a0 = 1.0; a1 = 0.0; a2 = 0.0;
            break;
    }
    c[0] = b0 / a0;
    c[1] = b1 / a0;
    c[2] = b2 / a0;
    c[3] = a1 / a0;
    c[4] = a2 / a0;
}

/* انتخاب s به‌طوری که C_max در [2^14, 2^15) قرار گیرد:
 *   s = -1 - ceil(log2(max_abs)) ،  C = round(c * 2^(15+s))
 * این کار تا 15 بیت دقت ضرایب می‌دهد و برای ورودی‌های ≤ ~0.5 تمام‌مقیاس،
 * سرریز accumulator را تضمیناً از بین می‌برد (توضیح در مستندات). */
static void dsp_biquad_quant(const double c[5], int32_t *qc, uint8_t *shift, int bits)
{
    double maxa = 0.0;
    int i, s;
    double scale;

    for (i = 0; i < 5; i++) {
        double a = fabs(c[i]);
        if (a > maxa) maxa = a;
    }
    if (maxa <= 0.0) maxa = 1.0;

    {
        double l2 = log(maxa) / log(2.0);
        int ce = (int)ceil(l2);
        s = -1 - ce;
    }
    if (s < -bits) s = -bits;   /* محافظ برای مقادیر خیلی بزرگ */

    scale = exp2((double)(bits - 1 + s));   /* 2^(15+s) یا 2^(31+s) */
    for (i = 0; i < 5; i++) {
        double v = c[i] * scale;
        if (v >= 0.0) v += 0.5; else v -= 0.5;
        if (v > 2147483647.0) v = 2147483647.0;
        if (v < -2147483648.0) v = -2147483648.0;
        qc[i] = (int32_t)v;
    }
    *shift = (uint8_t)((bits - 1) + s);   /* 15+s یا 31+s */
}

void dsp_biquad_quant_q15(const double c[5], dsp_q15_t *qc, uint8_t *shift)
{
    int32_t tmp[5];
    dsp_biquad_quant(c, tmp, shift, 16);
    for (int i = 0; i < 5; i++) qc[i] = (dsp_q15_t)tmp[i];
}

void dsp_biquad_quant_q31(const double c[5], dsp_q31_t *qc, uint8_t *shift)
{
    int32_t tmp[5];
    dsp_biquad_quant(c, tmp, shift, 32);
    for (int i = 0; i < 5; i++) qc[i] = (dsp_q31_t)tmp[i];
}

#endif /* DSP_ENABLE_BIQUAD */
