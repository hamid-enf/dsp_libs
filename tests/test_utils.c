/* تست ماژول‌های کاربردی: Moving / Median / Savgol / DCBlock / Notch / Utils */
#include "test_util.h"
#include "dsp.h"

static void test_moving(void)
{
    TEST_SECTION("Moving Average: SMA/WMA/EMA");
#if DSP_ENABLE_MOVING_AVERAGE
    dsp_sma_f32_t sma;
    dsp_f32_t buf[4];
    dsp_f32_t x[8] = {1,2,3,4,5,6,7,8}, y;
    TCHECK(dsp_sma_init_f32(&sma, 4, buf) == DSP_OK, "sma init");
    for (int i = 0; i < 8; i++) {
        dsp_sma_process_f32(&sma, x[i], &y);
        if (i == 3) TCHECK_NEAR(y, 2.5, 1e-5, "sma y3");
        if (i == 7) TCHECK_NEAR(y, 6.5, 1e-5, "sma y7");
    }
    /* dsp_wma: وزن‌های [1,3] */
    {
        dsp_wma_f32_t wma;
        dsp_f32_t w[2] = {1,3}, wbuf[2];
        dsp_wma_init_f32(&wma, 2, w, wbuf);
        dsp_wma_process_f32(&wma, 1, &y);   /* y = 0.25 */
        TCHECK_NEAR(y, 0.25, 1e-6, "wma y0");
        dsp_wma_process_f32(&wma, 3, &y);   /* (1*3 + 3*1)/4 = 1.5 */
        TCHECK_NEAR(y, 1.5, 1e-6, "wma y1");
    }
    /* EMA: همگرایی به ورودی ثابت */
    {
        dsp_ema_f32_t ema;
        dsp_ema_init_f32(&ema, 0.1f);
        for (int i = 0; i < 500; i++) dsp_ema_process_f32(&ema, 5.0f, &y);
        TCHECK_NEAR(y, 5.0, 1e-3, "ema steady");
    }
#endif
}

static void test_median(void)
{
    TEST_SECTION("Median: حذف Spike");
#if DSP_ENABLE_MEDIAN
    dsp_median_f32_t med;
    dsp_f32_t buf[5], sorted[5];
    dsp_f32_t x[7] = {1, 2, 100, 3, 4, 5, 6}, y;
    TCHECK(dsp_median_init_f32(&med, 5, buf, sorted) == DSP_OK, "med init");
    TCHECK(dsp_median_init_f32(&med, 4, buf, sorted) == DSP_ERR_INVALID_PARAMETER, "even window");
    dsp_median_init_f32(&med, 5, buf, sorted);
    for (int i = 0; i < 7; i++) dsp_median_process_f32(&med, x[i], &y);
    /* آخرین پنجره: {2,100,3,4,5} → میانه 4 */
    /* آخرین پنجره: {100,3,4,5,6} → میانه 5 */
    TCHECK_NEAR(y, 5.0, 1e-6, "median removes spike");
    /* Q15 */
    dsp_median_q15_t medq;
    dsp_q15_t bq[5], sq[5];
    dsp_q15_t xq[7] = {1000, 2000, 30000, 3000, 4000, 5000, 6000}, yq;
    dsp_median_init_q15(&medq, 5, bq, sq);
    for (int i = 0; i < 7; i++) dsp_median_process_q15(&medq, xq[i], &yq);
    TCHECK(yq == 5000, "median q15");
#endif
}

static void test_savgol(void)
{
    TEST_SECTION("Savitzky-Golay: هموارسازی سیگنال خطی");
#if DSP_ENABLE_SAVGOL
    /* سیگنال خطی + نویز: SG مرتبه 2 باید خط را بدون اعوجاج عبور دهد */
    dsp_f32_t coeffs[7];
    TCHECK(dsp_savgol_design_f32(3, 2, 0, coeffs) == DSP_OK, "sg design");
    /* مجموع ضرایب smoothing باید 1 باشد */
    {
        dsp_f32_t s = 0;
        for (int i = 0; i < 7; i++) s += coeffs[i];
        TCHECK_NEAR(s, 1.0, 1e-4, "sg coeff sum = 1");
    }
    dsp_savgol_f32_t sg;
    dsp_f32_t buf[7];
    dsp_savgol_init_f32(&sg, 3, coeffs, buf);
    {
        dsp_f32_t y;
        int i;
        for (i = 0; i < 30; i++) dsp_savgol_process_f32(&sg, 2.0f + 0.5f * i, &y);
        /* بعد از warm-up، خروجی باید همان خط باشد */
        TCHECK_NEAR(y, 2.0f + 0.5f * (29 - 3), 1e-3, "sg preserves line");
    }
    /* مشتق مرتبه 1 — برای خط با شیب 0.5 باید 0.5 بدهد */
    {
        dsp_f32_t dcoeffs[7];
        dsp_savgol_design_f32(3, 2, 1, dcoeffs);
        dsp_savgol_f32_t sg2;
        dsp_f32_t buf2[7];
        dsp_savgol_init_f32(&sg2, 3, dcoeffs, buf2);
        dsp_f32_t y;
        int i;
        for (i = 0; i < 30; i++) dsp_savgol_process_f32(&sg2, 2.0f + 0.5f * i, &y);
        TCHECK_NEAR(y, 0.5, 1e-3, "sg derivative = slope");
    }
#endif
}

static void test_dcblock(void)
{
    TEST_SECTION("DC Blocker");
#if DSP_ENABLE_DCBLOCK
    dsp_dcblock_f32_t dc;
    dsp_dcblock_init_f32(&dc, 0.995f);
    {
        dsp_f32_t y = 0, hist[2000];
        int i;
        for (i = 0; i < 2000; i++) {
            dsp_dcblock_process_f32(&dc, 1.0f + 0.1f * sinf(0.1f * i), &y);
            hist[i] = y;
        }
        /* مؤلفه DC باید حذف شود → میانگین ۱۰۰۰ نمونه‌ی آخر ≈ 0 */
        dsp_f32_t mean = 0;
        for (i = 1000; i < 2000; i++) mean += hist[i] / 1000.0f;
        TCHECK_NEAR(mean, 0.0, 2e-2, "dc removed");
    }
#endif
}

static void test_notch(void)
{
    TEST_SECTION("Notch 50Hz");
#if DSP_ENABLE_NOTCH
    dsp_notch_f32_t nf;
    dsp_notch_init_f32(&nf, 48000, 50, 30);
    /* پاسخ به سینوسی 50Hz باید ~0 و 100Hz باید ~1 باشد */
    {
        dsp_f32_t y50 = 0, y100 = 0, x;
        int i;
        dsp_notch_reset_f32(&nf);
        for (i = 0; i < 48000; i++) {
            x = sinf(2*3.14159265f*50*i/48000.0f);
            dsp_notch_process_f32(&nf, x, &y50);
        }
        dsp_notch_reset_f32(&nf);
        {
            dsp_f32_t max100 = 0;
            for (i = 0; i < 48000; i++) {
                x = sinf(2*3.14159265f*100*i/48000.0f);
                dsp_notch_process_f32(&nf, x, &y100);
                if (i > 47000) { dsp_f32_t a = fabsf(y100); if (a > max100) max100 = a; }
            }
            TCHECK(max100 > 0.9f, "100Hz passes");
        }
    }
    /* تغییر Runtime فرکانس */
    TCHECK(dsp_notch_set_freq_f32(&nf, 48000, 60, 30) == DSP_OK, "retune to 60Hz");
#endif
}

static void test_signal_utils(void)
{
    TEST_SECTION("Signal Utils: RMS/Peak/Envelope/Limiter");
#if DSP_ENABLE_SIGNAL_UTILS
    dsp_f32_t x[8] = {1, -1, 1, -1, 1, -1, 1, -1};
    dsp_f32_t rms = dsp_rms_f32(x, 8);
    TCHECK_NEAR(rms, 1.0, 1e-5, "rms of ±1");

    dsp_f32_t g[8];
    dsp_gain_f32(x, g, 8, 2.0f, 1);
    for (int i = 0; i < 8; i++) TCHECK_NEAR(fabsf(g[i]), 1.0, 1e-6, "gain saturates");

    dsp_f32_t peak = dsp_normalize_f32((dsp_f32_t[]){0.5f, 2.0f, -1.0f}, g, 3);
    TCHECK_NEAR(peak, 2.0, 1e-6, "normalize peak");
    TCHECK_NEAR(g[1], 1.0, 1e-6, "normalize scales");

    dsp_peak_f32_t pk;
    dsp_peak_init_f32(&pk, 0.5f, 0.9f);
    dsp_f32_t y;
    dsp_peak_process_f32(&pk, 0.8f, &y);
    TCHECK_NEAR(y, 0.4, 1e-4, "peak attack");
    dsp_peak_process_f32(&pk, 0.0f, &y);
    TCHECK(y < 0.4f, "peak release");

    dsp_limiter_f32_t lim;
    TCHECK(dsp_limiter_init_f32(&lim, 0.5f, 0.9f, 0.5f, 0.05f) == DSP_OK, "limiter init");
    /* چند نمونه‌ی پیاپی پرشدت — باید گین به سقف برسد */
    for (int k = 0; k < 50; k++) dsp_limiter_process_f32(&lim, 2.0f, &y);
    TCHECK(y <= 0.9f + 0.05f, "limiter caps output");
    TCHECK(y > 0.85f, "limiter reaches ceiling");
    /* ورودی کم‌شدت — رهاسازی به گین 1 */
    for (int k = 0; k < 500; k++) dsp_limiter_process_f32(&lim, 0.1f, &y);
    TCHECK_NEAR(y, 0.1, 0.05, "limiter releases");
#endif
}

void dsp_test_utils_all(void)
{
    test_moving();
    test_median();
    test_savgol();
    test_dcblock();
    test_notch();
    test_signal_utils();
}
