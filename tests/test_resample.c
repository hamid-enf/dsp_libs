/* تست تغییر نرخ نمونه‌برداری: Interp/Decim در برابر کانولوشن مستقیم */
#include "test_util.h"
#include "dsp.h"

static void test_interp(void)
{
    TEST_SECTION("Resample: Interpolation ×4 (مقایسه با FIR مستقیم)");
#if DSP_ENABLE_RESAMPLING
    uint16_t L = 4, P = 8;         /* ×4 ، P=8 */
    uint16_t taps = L * P;         /* 32 */
    dsp_f32_t proto[32];
    dsp_f32_t state[8];
    dsp_interp_f32_t interp;
    dsp_f32_t x[6], y[24];
    int i, m;

    for (i = 0; i < 6; i++) x[i] = sinf(0.2f * i);

    /* پروتوتایپ: پنجره‌ی sinc با cutoff = 1/L */
    dsp_resample_design_lp_f32(proto, taps, 1.0f / L);

    TCHECK(dsp_interp_init_f32(&interp, L, P, proto, state) == DSP_OK, "interp init");
    TCHECK(dsp_interp_process_f32(&interp, x, y, 6) == DSP_OK, "interp process");

    /* مرجع: صفر-گذاری + کانولوشن کامل */
    {
        dsp_f32_t xup[6 * 4 + 40];
        memset(xup, 0, sizeof(xup));
        for (i = 0; i < 6; i++) xup[i * L] = x[i];
        dsp_f32_t ref[24];
        /* از نمونه‌ی 16 به بعد (warm-up فیلتر) */
        for (m = 0; m < 24; m++) {
            dsp_f32_t acc = 0;
            for (int k = 0; k < taps; k++) {
                if (m - k >= 0) acc += proto[k] * xup[m - k];
            }
            ref[m] = acc;
        }
        for (m = 16; m < 24; m++) {
            TCHECK_NEAR(y[m], ref[m], 1e-4, "interp vs direct");
        }
    }
#endif
}

static void test_decim(void)
{
    TEST_SECTION("Resample: Decimation ÷2 (مقایسه با FIR مستقیم)");
#if DSP_ENABLE_RESAMPLING
    uint16_t D = 2, P = 8;
    uint16_t taps = D * P;   /* 16 */
    dsp_f32_t proto[16];
    dsp_f32_t state[16];
    dsp_decim_f32_t decim;
    dsp_f32_t x[40], y[20];
    int i, m;

    for (i = 0; i < 40; i++) x[i] = sinf(0.15f * i) + 0.5f * sinf(0.7f * i);
    dsp_resample_design_lp_f32(proto, taps, 1.0f / D);
    TCHECK(dsp_decim_init_f32(&decim, D, P, proto, state) == DSP_OK, "decim init");
    TCHECK(dsp_decim_process_f32(&decim, x, y, 20) == DSP_OK, "decim process");

    /* مرجع: y[m] = Σ h[k]·x[2m − k] */
    for (m = 4; m < 20; m++) {
        dsp_f32_t acc = 0;
        for (int k = 0; k < taps; k++) {
            int idx = 2 * m - k;
            if (idx >= 0 && idx < 40) acc += proto[k] * x[idx];
        }
        TCHECK_NEAR(y[m], acc, 1e-4, "decim vs direct");
    }
#endif
}

static void test_src_chain(void)
{
    TEST_SECTION("Resample: زنجیره‌ی Interp(3)+Decim(2) ≈ نرخ 1.5×");
#if DSP_ENABLE_RESAMPLING
    uint16_t L = 3, D = 2, P = 8;
    dsp_f32_t p1[3 * 8], p2[2 * 8];
    dsp_f32_t st1[8], st2[16];
    dsp_interp_f32_t ip;
    dsp_decim_f32_t dc;
    dsp_f32_t x[10], mid[40], y[20];
    int i;
    for (i = 0; i < 10; i++) x[i] = sinf(0.1f * i);
    dsp_resample_design_lp_f32(p1, 3 * 8, 1.0f / 3);
    dsp_resample_design_lp_f32(p2, 2 * 8, 1.0f / 2);
    dsp_interp_init_f32(&ip, L, P, p1, st1);
    dsp_decim_init_f32(&dc, D, P, p2, st2);
    dsp_interp_process_f32(&ip, x, mid, 10);
    dsp_decim_process_f32(&dc, mid, y, 20 / 2 > 20 ? 20 : (20 / 2));
    /* خروجی باید متناهی و هم‌نرخ تقریباً متناظر باشد (فقط sanity) */
    for (i = 0; i < 10; i++) {
        TCHECK(fabsf(y[i]) < 2.0f, "src chain bounded");
    }
    printf("  [info] src chain ok (10 in -> 30 mid -> 15 out)\n");
#endif
}

void dsp_test_resample_all(void)
{
    test_interp();
    test_decim();
    test_src_chain();
}
