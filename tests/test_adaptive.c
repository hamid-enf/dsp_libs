/* تست تطبیقی: LMS/NLMS/RLS + Kalman */
#include "test_util.h"
#include "dsp.h"

static void test_lms_identify(void)
{
    TEST_SECTION("LMS: شناسایی سیستم (System Identification)");
#if DSP_ENABLE_LMS
    dsp_f32_t w[16], xd[16];
    dsp_lms_f32_t lms;
    /* سیستم هدف: h = [0.5, -0.25, 0.1] (FIR 3-تایی) */
    dsp_f32_t ht[3] = {0.5f, -0.25f, 0.1f};
    dsp_f32_t y, e;
    int n;
    TCHECK(dsp_lms_init_f32(&lms, 16, 0.01f, 0.0f, w, xd) == DSP_OK, "lms init");
    /* سیگنال ورودی: نویز سفید + همبسته */
    dsp_f32_t xhist[10000];
    unsigned s = 12345;
    for (n = 0; n < 10000; n++) {
        s = s * 1103515245u + 12345u;
        xhist[n] = ((dsp_f32_t)(s >> 16) / 32768.0f) - 1.0f;
    }
    /* 5000 نمونه‌ی اول: شناسایی؛ بعد: خطای باقی‌مانده */
    dsp_f32_t mse_last = 0;
    for (n = 0; n < 9000; n++) {
        dsp_f32_t x = xhist[n];
        dsp_f32_t d = ht[0]*x + (n>0?ht[1]*xhist[n-1]:0) + (n>1?ht[2]*xhist[n-2]:0);
        dsp_lms_process_f32(&lms, x, d, &y, &e);
        if (n > 8800) mse_last += e * e;
    }
    mse_last /= 200;
    printf("  [info] LMS final MSE: %.6f (weights: %.3f %.3f %.3f)\n", mse_last, w[0], w[1], w[2]);
    TCHECK(mse_last < 0.001, "lms converged");
    TCHECK_NEAR(w[0], 0.5, 0.05, "lms w0");
    TCHECK_NEAR(w[1], -0.25, 0.05, "lms w1");
    TCHECK_NEAR(w[2], 0.1, 0.05, "lms w2");
#endif
}

static void test_nlms(void)
{
    TEST_SECTION("NLMS: شناسایی سیستم با توان متغیر");
#if DSP_ENABLE_NLMS
    dsp_f32_t w[16], xd[16];
    dsp_nlms_f32_t nlms;
    dsp_f32_t ht[3] = {0.5f, -0.25f, 0.1f};
    dsp_f32_t y, e, mse = 0;
    int n;
    unsigned s = 999;
    dsp_f32_t xprev[2] = {0, 0};
    dsp_nlms_init_f32(&nlms, 16, 0.5f, 1e-6f, w, xd);
    for (n = 0; n < 6000; n++) {
        dsp_f32_t x;
        s = s * 1103515245u + 12345u;
        x = ((dsp_f32_t)(s >> 16) / 32768.0f) - 1.0f;
        if (n > 3000) x *= 10.0f;   /* تغییر ناگهانی توان — تست عادی‌سازی */
        {
            dsp_f32_t d = ht[0]*x + ht[1]*xprev[0] + ht[2]*xprev[1];
            dsp_nlms_process_f32(&nlms, x, d, &y, &e);
        }
        xprev[1] = xprev[0]; xprev[0] = x;
        if (n > 5900) mse += e * e;
    }
    mse /= 100;
    printf("  [info] NLMS final MSE (with x10 power change): %.6f\n", mse);
    TCHECK(mse < 0.005, "nlms converged with power change");
#endif
}

static void test_rls(void)
{
    TEST_SECTION("RLS: شناسایی سیستم (همگرایی سریع)");
#if DSP_ENABLE_RLS
    dsp_f32_t w[8], xd[8], P[64];
    dsp_rls_f32_t rls;
    dsp_f32_t ht[3] = {0.5f, -0.25f, 0.1f};
    dsp_f32_t y, e, mse = 0;
    int n;
    unsigned s = 777;
    static dsp_f32_t xprev[2] = {0, 0};
    dsp_rls_init_f32(&rls, 8, 0.99f, 10.0f, w, xd, P);
    for (n = 0; n < 800; n++) {
        dsp_f32_t x;
        s = s * 1103515245u + 12345u;
        x = ((dsp_f32_t)(s >> 16) / 32768.0f) - 1.0f;
        {
            dsp_f32_t d = ht[0]*x + ht[1]*xprev[0] + ht[2]*xprev[1];
            dsp_rls_process_f32(&rls, x, d, &y, &e);
        }
        xprev[1] = xprev[0]; xprev[0] = x;
        if (n > 750) mse += e * e;
    }
    mse /= 50;
    printf("  [info] RLS final MSE after 800 iters: %.8f\n", mse);
    TCHECK(mse < 1e-4, "rls fast convergence");
    TCHECK_NEAR(w[0], 0.5, 0.02, "rls w0");
    TCHECK_NEAR(w[1], -0.25, 0.02, "rls w1");
#endif
}

static void test_kalman(void)
{
    TEST_SECTION("Kalman 1D: تخمین مقدار ثابت در نویز");
#if DSP_ENABLE_KALMAN
    dsp_kalman1d_f32_t kf;
    dsp_f32_t x = 10.0f, z;
    int i;
    unsigned s = 42;
    dsp_kalman1d_init_f32(&kf, 0.0f, 1.0f, 1e-4f, 0.25f);
    for (i = 0; i < 400; i++) {
        s = s * 1103515245u + 12345u;
        z = 10.0f + (((dsp_f32_t)(s >> 16) / 32768.0f) - 1.0f) * 0.5f;  /* نویز ±0.5 */
        dsp_kalman1d_process_f32(&kf, z, &x);
    }
    printf("  [info] Kalman estimate: %.4f (true 10.0)\n", x);
    TCHECK_NEAR(x, 10.0, 0.05, "kalman converges");
    /* گین باید به سمت مقدار کوچک برود */
    TCHECK(kf.k < 0.2f, "kalman gain settles");
#endif
}

void dsp_test_adaptive_all(void)
{
    test_lms_identify();
    test_nlms();
    test_rls();
    test_kalman();
}
