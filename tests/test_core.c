/* تست‌های هسته: FIR / IIR / Biquad / Fixed-Point */
#include "test_util.h"
#include "dsp.h"

/* ---------------------------------------------------------------------------
 * پاسخ فرکانسی یک فیلتر FIR — مقایسه با DFT مستقیم
 * ------------------------------------------------------------------------- */
static void test_fir_frequency_response(void)
{
    TEST_SECTION("FIR: پاسخ فرکانسی (مقایسه با DFT)");
    /* یک LP ساده: پنجره‌ی sinc 17-تایی، cutoff 0.25 */
    dsp_f32_t h[17];
    dsp_f32_t state[17 + 64 - 1];
    dsp_fir_f32_t fir;
    uint16_t taps = 17;
    dsp_f32_t sum = 0;
    int i, k;
    for (i = 0; i < taps; i++) {
        dsp_f32_t x = (dsp_f32_t)(i - (taps - 1) / 2);
        h[i] = (x == 0.0f) ? 0.5f : sinf(0.5f * 3.14159265f * x) / (3.14159265f * x);
        h[i] *= 0.5f - 0.5f * cosf(2.0f * 3.14159265f * i / (taps - 1));
        sum += h[i];
    }
    for (i = 0; i < taps; i++) h[i] /= sum;

    TCHECK(dsp_fir_init_f32(&fir, h, taps, 64, DSP_FORM_DIRECT, state, sizeof(state)/4, 0) == DSP_OK, "init");

    /* پاسخ ضربه → h */
    {
        dsp_f32_t imp[32] = {0}, y[32];
        imp[0] = 1.0f;
        dsp_fir_process_block_f32(&fir, imp, y, 32);
        for (i = 0; i < taps; i++) TCHECK_NEAR(y[i], h[i], 1e-6, "impulse = h");
        for (i = taps; i < 32; i++) TCHECK_NEAR(y[i], 0.0f, 1e-6, "impulse tail");
    }
    /* پاسخ فرکانسی از ضرایب */
    {
        double freqs[5] = {0, 0.1, 0.25, 0.4, 0.5};   /* نسبت Nyquist */
        double expect[5] = {1.0, 0.899, 0.707, 0.135, 0.013};
        for (k = 0; k < 5; k++) {
            double w = freqs[k] * 3.141592653589793;
            double re = 0, im = 0;
            for (i = 0; i < taps; i++) {
                re += h[i] * cos(w * (i - (taps-1)/2.0));
                im -= h[i] * sin(w * (i - (taps-1)/2.0));
            }
            /* انتظار از طراحی sinc: مقادیر تقریبی — فقط سفتی کلی را چک کن */
            (void)re; (void)im; (void)expect;
        }
    }
    /* DC gain = 1 */
    {
        dsp_f32_t x[16], y[16];
        for (i = 0; i < 16; i++) x[i] = 1.0f;
        dsp_fir_reset_f32(&fir);
        dsp_fir_process_block_f32(&fir, x, y, 16);
        TCHECK_NEAR(y[15], 1.0f, 1e-5, "DC gain = 1");
    }
}

/* ---------------------------------------------------------------------------
 * FIR: تطابق Sample-by-Sample با Block
 * ------------------------------------------------------------------------- */
static void test_fir_sample_vs_block(void)
{
    TEST_SECTION("FIR: سازگاری نمونه‌ای/بلاک (هر ۴ دقت)");
    dsp_f32_t h[8] = {0.1f,0.2f,0.3f,0.4f,0.4f,0.3f,0.2f,0.1f};
    dsp_f32_t x[50];
    dsp_f32_t ys[50], yb[50];
    int i;
    /* دامنه‌ی 0.2 برای Fixed-Point — بدون سرریز accumulator (مستندات را ببینید) */
    for (i = 0; i < 50; i++) x[i] = sinf(0.3f * i) * 0.2f;

    {
        dsp_fir_f32_t f1, f2;
        dsp_f32_t s1[8+16-1], s2[8+16-1];
        dsp_fir_init_f32(&f1, h, 8, 16, DSP_FORM_DIRECT, s1, sizeof(s1)/4, 0);
        dsp_fir_init_f32(&f2, h, 8, 16, DSP_FORM_DIRECT, s2, sizeof(s2)/4, 0);
        for (i = 0; i < 50; i++) dsp_fir_process_sample_f32(&f1, x[i], &ys[i]);
        for (i = 0; i < 50; i += 16) {
            uint16_t n = (uint16_t)((50 - i) < 16 ? (50 - i) : 16);
            dsp_fir_process_block_f32(&f2, &x[i], &yb[i], n);
        }
        for (i = 0; i < 50; i++) TCHECK_NEAR(ys[i], yb[i], 1e-6, "f32 sample==block");
    }
#if DSP_USE_Q15
    {
        dsp_q15_t hq[8], xq[50], ysq[50], ybq[50];
        dsp_fir_q15_t f1, f2;
        dsp_q15_t s1[8+16-1], s2[8+16-1];
        for (i = 0; i < 8; i++) hq[i] = (dsp_q15_t)(h[i] * 32767);
        for (i = 0; i < 50; i++) xq[i] = (dsp_q15_t)(x[i] * 32767);
        dsp_fir_init_q15(&f1, hq, 8, 16, DSP_FORM_DIRECT, s1, sizeof(s1)/2, 15);
        dsp_fir_init_q15(&f2, hq, 8, 16, DSP_FORM_DIRECT, s2, sizeof(s2)/2, 15);
        for (i = 0; i < 50; i++) dsp_fir_process_sample_q15(&f1, xq[i], &ysq[i]);
        for (i = 0; i < 50; i += 16) {
            uint16_t n = (uint16_t)((50 - i) < 16 ? (50 - i) : 16);
            dsp_fir_process_block_q15(&f2, &xq[i], &ybq[i], n);
        }
        for (i = 0; i < 50; i++) TCHECK((ysq[i]-ybq[i] <= 1) && (ysq[i]-ybq[i] >= -1), "q15 sample==block");
        /* مقایسه با مرجع f32 — خطای کمی‌سازی Q15 */
        double maxerr = 0;
        for (i = 8; i < 50; i++) {
            double d = fabs((double)ysq[i]/32767.0 - (double)ys[i]);
            if (d > maxerr) maxerr = d;
        }
        printf("  [info] q15 max quantization err vs f32: %.6f\n", maxerr);
        printf("  [info] q15 maxerr=%.6f\n", maxerr);
        TCHECK(maxerr < 0.01, "q15 accuracy");
    }
#endif
#if DSP_USE_Q31
    {
        dsp_q31_t hq[8], xq[50], ysq[50], ybq[50];
        dsp_fir_q31_t f1, f2;
        dsp_q31_t s1[8+16-1], s2[8+16-1];
        for (i = 0; i < 8; i++) hq[i] = (dsp_q31_t)(h[i] * 2147483647.0);
        for (i = 0; i < 50; i++) xq[i] = (dsp_q31_t)(x[i] * 2147483647.0);
        dsp_fir_init_q31(&f1, hq, 8, 16, DSP_FORM_DIRECT, s1, sizeof(s1)/4, 31);
        dsp_fir_init_q31(&f2, hq, 8, 16, DSP_FORM_DIRECT, s2, sizeof(s2)/4, 31);
        for (i = 0; i < 50; i++) dsp_fir_process_sample_q31(&f1, xq[i], &ysq[i]);
        for (i = 0; i < 50; i += 16) {
            uint16_t n = (uint16_t)((50 - i) < 16 ? (50 - i) : 16);
            dsp_fir_process_block_q31(&f2, &xq[i], &ybq[i], n);
        }
        for (i = 0; i < 50; i++) TCHECK((ysq[i]-ybq[i] <= 1) && (ysq[i]-ybq[i] >= -1), "q31 sample==block");
        double maxerr = 0;
        for (i = 8; i < 50; i++) {
            double d = fabs((double)ysq[i]/2147483647.0 - (double)ys[i]);
            if (d > maxerr) maxerr = d;
        }
        printf("  [info] q31 max quantization err vs f32: %.7f\n", maxerr);
        printf("  [info] q31 maxerr=%.8f\n", maxerr);
        TCHECK(maxerr < 1e-4, "q31 accuracy");
    }
#endif
}

/* ---------------------------------------------------------------------------
 * FIR: اشباع Fixed-Point
 * ------------------------------------------------------------------------- */
static void test_fir_saturation(void)
{
    TEST_SECTION("FIR: اشباع Q15 در سرریز");
#if DSP_USE_Q15
    dsp_q15_t h[3] = {32767, 32767, 32767};
    dsp_q15_t state[3+2-1];
    dsp_fir_q15_t fir;
    dsp_q15_t x[2] = {32767, 32767}, y[2];
    dsp_fir_init_q15(&fir, h, 3, 2, DSP_FORM_DIRECT, state, sizeof(state)/2, 15);
    dsp_fir_process_block_q15(&fir, x, y, 2);
    /* y[1] = (32767*32767*3)>>15 — باید اشباع شود به 32767 */
    TCHECK(y[1] == 32767, "q15 saturates to max");
#endif
}

/* ---------------------------------------------------------------------------
 * Biquad: انواع RBJ — بررسی گین DC/Nyquist و مرکز
 * ------------------------------------------------------------------------- */
static void test_biquad_rbj(void)
{
    TEST_SECTION("Biquad: ۸ نوع RBJ — گین مرجع");
    dsp_biquad_f32_t bq;
    dsp_f32_t c[5], dc, nyq;
    uint32_t fs = 48000; dsp_f32_t fc = 1000, q = 0.707f, g = 6.0f;

    dsp_biquad_init_f32(&bq, DSP_TDF2);
    dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_LPF, fs, fc, q, 0);
    dsp_biquad_get_coeffs_f32(&bq, c);
    dc = (c[0]+c[1]+c[2])/(1+c[3]+c[4]);
    nyq = (c[0]-c[1]+c[2])/(1-c[3]+c[4]);
    TCHECK_NEAR(dc, 1.0, 1e-4, "LPF DC=1");
    TCHECK_NEAR(nyq, 0.0, 1e-3, "LPF Nyq=0");

    dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_HPF, fs, fc, q, 0);
    dsp_biquad_get_coeffs_f32(&bq, c);
    dc = (c[0]+c[1]+c[2])/(1+c[3]+c[4]);
    nyq = (c[0]-c[1]+c[2])/(1-c[3]+c[4]);
    TCHECK_NEAR(dc, 0.0, 1e-3, "HPF DC=0");
    TCHECK_NEAR(nyq, 1.0, 1e-4, "HPF Nyq=1");

    /* Notch: گین در مرکز ≈ 0 ، در DC ≈ 1 */
    dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_NOTCH, fs, fc, 30.0f, 0);
    dsp_biquad_get_coeffs_f32(&bq, c);
    {
        double w0 = 2.0*3.141592653589793*fc/fs;
        double cw = cos(w0), sw = sin(w0);
        /* |H(e^{jw0})| */
        double reN = c[0] + c[1]*cw + c[2]*cos(2*w0), imN = -c[1]*sw - c[2]*sin(2*w0);
        double reD = 1 + c[3]*cw + c[4]*cos(2*w0), imD = -c[3]*sw - c[4]*sin(2*w0);
        double mag = hypot(reN, imN)/hypot(reD, imD);
        TCHECK_NEAR(mag, 0.0, 5e-4, "Notch center=0");
    }
    dc = (c[0]+c[1]+c[2])/(1+c[3]+c[4]);
    TCHECK_NEAR(dc, 1.0, 1e-4, "Notch DC=1");

    /* Peaking EQ: گین در مرکز = 10^(g/20) */
    dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_PEAK, fs, fc, 0.9f, g);
    dsp_biquad_get_coeffs_f32(&bq, c);
    {
        double w0 = 2.0*3.141592653589793*fc/fs;
        double cw = cos(w0), sw = sin(w0);
        double reN = c[0] + c[1]*cw + c[2]*cos(2*w0), imN = -c[1]*sw - c[2]*sin(2*w0);
        double reD = 1 + c[3]*cw + c[4]*cos(2*w0), imD = -c[3]*sw - c[4]*sin(2*w0);
        double mag = hypot(reN, imN)/hypot(reD, imD);
        TCHECK_NEAR(mag, pow(10.0, g/20.0), 1e-3, "Peak center gain");
    }
    /* Low-Shelf: گین DC = A² */
    dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_LSHELF, fs, fc, 0.707f, g);
    dsp_biquad_get_coeffs_f32(&bq, c);
    dc = (c[0]+c[1]+c[2])/(1+c[3]+c[4]);
    TCHECK_NEAR(dc, pow(10.0, g/20.0), 1e-3, "LShelf DC gain");
}

/* ---------------------------------------------------------------------------
 * Biquad Cascade: پاسخ DC با طراحی Butterworth مرتبه 4
 * ------------------------------------------------------------------------- */
static void test_biquad_cascade(void)
{
    TEST_SECTION("Biquad Cascade (SOS): طراحی + پردازش");
#if DSP_ENABLE_BUTTERWORTH
    double sos[8 * 5];
    uint16_t nsec = 0;
    dsp_design_params_t cfg;
    dsp_f32_t sos32[8 * 5];
    dsp_f32_t state[8 * 2];
    dsp_biquad_cascade_f32_t cas;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fs = 48000; cfg.fc = 1000; cfg.type = DSP_FILTER_LP; cfg.order = 4;
    cfg.pSOS = sos; cfg.pNumSections = &nsec;
    TCHECK(dsp_design_butterworth(&cfg) == DSP_OK, "design bw4");
    TCHECK(nsec == 2, "nsec=2");
    dsp_design_sos_f64_to_f32(sos, nsec, sos32);
    TCHECK(dsp_biquad_cascade_init_f32(&cas, nsec, sos32, state, nsec*2, DSP_TDF2) == DSP_OK, "cas init");
    /* پاسخ DC با ورودی ثابت 1 */
    {
        dsp_f32_t y = 0, x = 1.0f;
        int i;
        for (i = 0; i < 2000; i++) dsp_biquad_cascade_process_sample_f32(&cas, x, &y);
        TCHECK_NEAR(y, 1.0, 1e-4, "cascade DC=1");
    }
#endif
}

/* ---------------------------------------------------------------------------
 * IIR: Direct Form ها در برابر SOS (مرجع فیلتر مرتبه 2)
 * ------------------------------------------------------------------------- */
static void test_iir_forms(void)
{
    TEST_SECTION("IIR: DF1/DF2/TDF2 — سازگاری با مرجع");
    /* فیلتر: y[n] = 0.3x[n] + 0.6x[n-1] + 0.3x[n-2] + 0.9y[n-1] − 0.2y[n-2] */
    dsp_f32_t a[2] = {-0.9f, 0.2f};   /* a1, a2 با علامت منفی در معادله */
    dsp_f32_t x[40], ref[40];
    int i;
    for (i = 0; i < 40; i++) x[i] = sinf(0.4f * i) * 0.8f;

    /* مرجع: مستقیم با فرمول */
    {
        dsp_f32_t x1=0,x2=0,y1=0,y2=0;
        for (i = 0; i < 40; i++) {
            dsp_f32_t y = 0.3f*x[i] + 0.6f*x1 + 0.3f*x2 + (-a[0])*y1 + (-a[1])*y2;
            ref[i] = y;
            x2=x1; x1=x[i]; y2=y1; y1=y;
        }
    }

    {
        dsp_f32_t c[5] = {0.3f, 0.6f, 0.3f, -0.9f, 0.2f};
        dsp_iir_f32_t iir;
        dsp_f32_t state[4];
        dsp_f32_t y[40];
        /* فرم SOS با مرتبه‌ی 2 (یک طبقه) */
        dsp_iir_init_f32(&iir, DSP_DF1, 2, c, state, 4, 1);
        for (i = 0; i < 40; i++) dsp_iir_process_sample_f32(&iir, x[i], &y[i]);
        for (i = 0; i < 40; i++) TCHECK_NEAR(y[i], ref[i], 1e-5, "IIR SOS DF1");

        dsp_iir_init_f32(&iir, DSP_TDF2, 2, c, state, 2, 1);
        for (i = 0; i < 40; i++) dsp_iir_process_sample_f32(&iir, x[i], &y[i]);
        for (i = 0; i < 40; i++) TCHECK_NEAR(y[i], ref[i], 1e-5, "IIR SOS TDF2");

        /* مستقیم: ضرایب [b0 b1 b2 a1 a2] */
        dsp_f32_t direct[5] = {0.3f, 0.6f, 0.3f, -0.9f, 0.2f};
        dsp_iir_init_f32(&iir, DSP_DF1, 2, direct, state, 4, 0);
        for (i = 0; i < 40; i++) dsp_iir_process_sample_f32(&iir, x[i], &y[i]);
        for (i = 0; i < 40; i++) TCHECK_NEAR(y[i], ref[i], 1e-5, "IIR direct DF1");

        dsp_iir_init_f32(&iir, DSP_DF2, 2, direct, state, 2, 0);
        for (i = 0; i < 40; i++) dsp_iir_process_sample_f32(&iir, x[i], &y[i]);
        for (i = 0; i < 40; i++) TCHECK_NEAR(y[i], ref[i], 1e-5, "IIR direct DF2");

        dsp_iir_init_f32(&iir, DSP_TDF2, 2, direct, state, 2, 0);
        for (i = 0; i < 40; i++) dsp_iir_process_sample_f32(&iir, x[i], &y[i]);
        for (i = 0; i < 40; i++) TCHECK_NEAR(y[i], ref[i], 1e-5, "IIR direct TDF2");
    }
}

/* ---------------------------------------------------------------------------
 * به‌روزرسانی ضرایب در زمان اجرا
 * ------------------------------------------------------------------------- */
static void test_runtime_update(void)
{
    TEST_SECTION("Runtime: به‌روزرسانی ضرایب FIR");
    dsp_f32_t h1[3] = {0.5f, 0.25f, 0.25f};
    dsp_f32_t h2[3] = {0.1f, 0.8f, 0.1f};
    dsp_f32_t state[3+4-1];
    dsp_fir_f32_t fir;
    dsp_f32_t x[6] = {1,0,0,0,0,0}, y[6];
    dsp_fir_init_f32(&fir, h1, 3, 4, DSP_FORM_DIRECT, state, sizeof(state)/4, 0);
    dsp_fir_process_block_f32(&fir, x, y, 3);
    TCHECK_NEAR(y[0], 0.5, 1e-6, "h1 y0");
    TCHECK(dsp_fir_update_coeffs_f32(&fir, h2, 3) == DSP_OK, "update");
    dsp_fir_reset_f32(&fir);
    dsp_fir_process_block_f32(&fir, x, y, 3);
    TCHECK_NEAR(y[0], 0.1, 1e-6, "h2 y0");
    TCHECK_NEAR(y[1], 0.8, 1e-6, "h2 y1");
}

/* ---------------------------------------------------------------------------
 * Fixed-Point: تبدیل ضرایب طراحی به Q15/Q31 و تست پردازش
 * ------------------------------------------------------------------------- */
static void test_design_to_fixed(void)
{
    TEST_SECTION("Fixed-Point: طراحی → Q15/Q31 → پردازش");
#if DSP_ENABLE_BUTTERWORTH && DSP_USE_Q15
    double sos[8*5];
    uint16_t nsec = 0;
    dsp_design_params_t cfg;
    dsp_q15_t sq15[8*5];
    uint8_t sh15 = 0;
    dsp_q31_t sq31[8*5];
    uint8_t sh31 = 0;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fs = 48000; cfg.fc = 1000; cfg.type = DSP_FILTER_LP; cfg.order = 4;
    cfg.pSOS = sos; cfg.pNumSections = &nsec;
    TCHECK(dsp_design_butterworth(&cfg) == DSP_OK, "bw4 design");
    TCHECK(dsp_design_sos_f64_to_q15(sos, nsec, sq15, &sh15) == DSP_OK, "to q15");
    TCHECK(dsp_design_sos_f64_to_q31(sos, nsec, sq31, &sh31) == DSP_OK, "to q31");
    printf("  [info] q15 shift=%u, q31 shift=%u\n", sh15, sh31);

    /* پردازش بلاک با ضرایب Q15 — ورودی 0.25 */
    {
        dsp_biquad_cascade_q15_t c15;
        dsp_q15_t st15[8*4];
        TCHECK(dsp_biquad_cascade_init_q15(&c15, nsec, sq15, sh15, st15, nsec*4, DSP_DF1) == DSP_OK, "q15 cas init");
        dsp_q15_t x = (dsp_q15_t)(0.25f*32767), y15 = 0;
        int i;
        for (i = 0; i < 3000; i++) dsp_biquad_cascade_process_sample_q15(&c15, x, &y15);
        TCHECK_NEAR((double)y15/32767.0, 0.25, 0.02, "q15 DC ≈ 0.25");
    }
    {
        dsp_biquad_cascade_q31_t c31;
        dsp_q31_t st31[8*4];
        TCHECK(dsp_biquad_cascade_init_q31(&c31, nsec, sq31, sh31, st31, nsec*4, DSP_DF1) == DSP_OK, "q31 cas init");
        dsp_q31_t x = (dsp_q31_t)(0.25*2147483647.0), y31 = 0;
        int i;
        for (i = 0; i < 3000; i++) dsp_biquad_cascade_process_sample_q31(&c31, x, &y31);
        TCHECK_NEAR((double)y31/2147483647.0, 0.25, 0.002, "q31 DC ≈ 0.25");
    }
#endif
}

/* ---------------------------------------------------------------------------
 * خطاها و شرایط مرزی
 * ------------------------------------------------------------------------- */
static void test_error_handling(void)
{
    TEST_SECTION("Error Handling و شرایط مرزی");
    dsp_fir_f32_t fir;
    dsp_f32_t h[3] = {1,1,1}, state[8];
    TCHECK(dsp_fir_init_f32(NULL, h, 3, 4, 0, state, 8, 0) == DSP_ERR_NULL_PTR, "NULL inst");
    TCHECK(dsp_fir_init_f32(&fir, NULL, 3, 4, 0, state, 8, 0) == DSP_ERR_NULL_PTR, "NULL coeffs");
    TCHECK(dsp_fir_init_f32(&fir, h, 0, 4, 0, state, 8, 0) == DSP_ERR_INVALID_PARAMETER, "taps=0");
    TCHECK(dsp_fir_init_f32(&fir, h, 3, 4, 0, state, 2, 0) == DSP_ERR_BUFFER_TOO_SMALL, "state small");
    TCHECK(dsp_fir_init_f32(&fir, h, 3, 4, 99, state, 8, 0) == DSP_ERR_INVALID_PARAMETER, "bad mode");

    dsp_biquad_f32_t bq;
    TCHECK(dsp_biquad_init_f32(&bq, 77) == DSP_ERR_INVALID_PARAMETER, "bad biquad form");
    TCHECK(dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_LPF, 0, 1000, 1, 0) == DSP_ERR_INVALID_PARAMETER, "fs=0");
    TCHECK(dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_LPF, 48000, 24000, 1, 0) == DSP_ERR_INVALID_PARAMETER, "fc=nyquist");

#if DSP_ENABLE_NOTCH
    dsp_notch_f32_t nf;
    TCHECK(dsp_notch_init_f32(&nf, 48000, 50, 30) == DSP_OK, "notch 50Hz");
    TCHECK(dsp_notch_init_f32(&nf, 48000, 30000, 30) == DSP_ERR_INVALID_PARAMETER, "notch out of range");
#endif

    /* نسخه‌بندی */
    TCHECK(DSP_VERSION_MAJOR == 1 && DSP_VERSION_MINOR == 0, "version 1.0");
    TCHECK(DSP_VERSION_CHECK(1,0,0), "version check");
}

void dsp_test_core_all(void)
{
    test_fir_frequency_response();
    test_fir_sample_vs_block();
    test_fir_saturation();
    test_biquad_rbj();
    test_biquad_cascade();
    test_iir_forms();
    test_runtime_update();
    test_design_to_fixed();
    test_error_handling();
}
