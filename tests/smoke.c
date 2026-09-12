/* تست دود سریع — هسته‌ی کتابخانه */
#include <stdio.h>
#include <math.h>
#include "dsp.h"

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", msg, __LINE__); fails++; } } while (0)
#define CHECK_NEAR(a, b, tol, msg) do { double _d = fabs((double)(a)-(double)(b)); if (_d > (tol)) { printf("FAIL: %s : %g vs %g (diff %g)\n", msg, (double)(a), (double)(b), _d); fails++; } } while (0)

int main(void)
{
    /* ---- FIR f32: میانگین‌گیر 3-تایی (h = [1/3 1/3 1/3]) ---- */
    {
        dsp_f32_t h[3] = {1.0f/3, 1.0f/3, 1.0f/3};
        dsp_f32_t state[3+8-1];
        dsp_fir_f32_t fir;
        dsp_f32_t x[8] = {1,1,1,1,1,1,1,1};
        dsp_f32_t y[8];
        dsp_err_t e = dsp_fir_init_f32(&fir, h, 3, 8, DSP_FORM_DIRECT, state, 10, 0);
        CHECK(e == DSP_OK, "fir init");
        dsp_fir_process_block_f32(&fir, x, y, 8);
        CHECK_NEAR(y[0], 1.0f/3, 1e-6, "fir block y0");
        CHECK_NEAR(y[2], 1.0f, 1e-6, "fir block y2");
        /* نمونه‌ای — باید همان خروجی بلاک را بدهد */
        dsp_fir_reset_f32(&fir);
        for (int i = 0; i < 8; i++) dsp_fir_process_sample_f32(&fir, x[i], &y[i]);
        CHECK_NEAR(y[2], 1.0f, 1e-6, "fir sample y2");
        /* پاسخ ضربه: h */
        dsp_fir_reset_f32(&fir);
        for (int i = 0; i < 8; i++) {
            dsp_f32_t imp = (i == 0) ? 1.0f : 0.0f;
            dsp_fir_process_sample_f32(&fir, imp, &y[i]);
        }
        CHECK_NEAR(y[0], 1.0f/3, 1e-6, "fir impulse y0");
        CHECK_NEAR(y[1], 1.0f/3, 1e-6, "fir impulse y1");
        CHECK_NEAR(y[2], 1.0f/3, 1e-6, "fir impulse y2");
        CHECK_NEAR(y[3], 0.0f, 1e-7, "fir impulse y3");
    }

    /* ---- FIR transposed ---- */
    {
        dsp_f32_t h[3] = {0.5f, 0.25f, 0.25f};
        dsp_f32_t state[3];
        dsp_fir_f32_t fir;
        dsp_f32_t x[4] = {1, 2, 3, 4};
        dsp_f32_t y[4];
        dsp_err_t e = dsp_fir_init_f32(&fir, h, 3, 4, DSP_FORM_TRANSPOSED, state, 3, 0);
        CHECK(e == DSP_OK, "fir tr init");
        dsp_fir_process_block_f32(&fir, x, y, 4);
        /* y[0] = 0.5*1 = 0.5 ; y[1] = 0.5*2+0.25*1 = 1.25 ; y[2] = 0.5*3+0.25*2+0.25*1 = 2.25 ; y[3] = 0.5*4+0.25*3+0.25*2=3.25 */
        CHECK_NEAR(y[0], 0.5, 1e-6, "fir tr y0");
        CHECK_NEAR(y[1], 1.25, 1e-6, "fir tr y1");
        CHECK_NEAR(y[2], 2.25, 1e-6, "fir tr y2");
        CHECK_NEAR(y[3], 3.25, 1e-6, "fir tr y3");
    }

    /* ---- FIR symmetric ---- */
    {
        dsp_f32_t h[5] = {0.1f, 0.2f, 0.4f, 0.2f, 0.1f};
        dsp_f32_t state[16];
        dsp_fir_f32_t fir;
        dsp_f32_t x[6] = {1, 0, 0, 0, 0, 0};
        dsp_f32_t y[6];
        dsp_err_t e = dsp_fir_init_f32(&fir, h, 5, 6, DSP_FORM_SYMMETRIC, state, 16, 0);
        CHECK(e == DSP_OK, "fir sym init");
        dsp_fir_process_block_f32(&fir, x, y, 6);
        CHECK_NEAR(y[0], 0.1, 1e-6, "fir sym y0");
        CHECK_NEAR(y[2], 0.4, 1e-6, "fir sym y2");
        CHECK_NEAR(y[4], 0.1, 1e-6, "fir sym y4");
    }

    /* ---- Biquad LPF f32: تطبیق با فرمول مستقیم ---- */
    {
        dsp_biquad_f32_t bq;
        dsp_f32_t y;
        CHECK(dsp_biquad_init_f32(&bq, DSP_TDF2) == DSP_OK, "bq init");
        /* یک فیلتر ساده: y[n] = 0.5x[n] + 0.5y[n-1] */
        CHECK(dsp_biquad_set_coeffs_f32(&bq, 0.5f, 0.0f, 0.0f, -0.5f, 0.0f) == DSP_OK, "bq coeffs");
        dsp_biquad_process_sample_f32(&bq, 1.0f, &y);
        CHECK_NEAR(y, 0.5, 1e-6, "bq y0");
        dsp_biquad_process_sample_f32(&bq, 1.0f, &y);
        CHECK_NEAR(y, 0.75, 1e-6, "bq y1");
        dsp_biquad_process_sample_f32(&bq, 0.0f, &y);
        CHECK_NEAR(y, 0.375, 1e-6, "bq y2");
    }

    /* ---- Biquad set_params: LPF با RBJ، پاسخ DC باید 1 باشد ---- */
    {
        dsp_biquad_f32_t bq;
        dsp_f32_t c[5], dc = 0;
        CHECK(dsp_biquad_init_f32(&bq, DSP_TDF2) == DSP_OK, "bq2 init");
        CHECK(dsp_biquad_set_params_f32(&bq, DSP_BIQUAD_LPF, 48000, 1000, 0.707f, 0) == DSP_OK, "bq2 params");
        dsp_biquad_get_coeffs_f32(&bq, c);
        dc = (c[0]+c[1]+c[2])/(1.0f+c[3]+c[4]);
        CHECK_NEAR(dc, 1.0, 1e-4, "bq2 DC gain");
    }

    /* ---- IIR SOS: Butterworth order 2 از scipy: [[0.06745527, 0.13491055, 0.06745527, 1, -0.63444844, 0.59182641]] */
    {
        dsp_f32_t sos[5] = {0.06745527f, 0.13491055f, 0.06745527f, -0.63444844f, 0.59182641f};
        dsp_f32_t state[2];
        dsp_iir_f32_t iir;
        dsp_f32_t y;
        CHECK(dsp_iir_init_f32(&iir, DSP_TDF2, 2, sos, state, 2, 1) == DSP_OK, "iir init");
        /* پاسخ DC: H(1) = (b0+b1+b2)/(1+a1+a2) = 0.26982/(1-0.63445+0.59183) = 0.26982/0.95738 = 0.28183 */
        dsp_f32_t x = 1.0f;
        for (int i = 0; i < 200; i++) dsp_iir_process_sample_f32(&iir, x, &y);
        CHECK_NEAR(y, 0.28183, 1e-4, "iir DC steady");
    }

    /* ---- Fixed point: FIR q15 ---- */
    {
        dsp_q15_t h[3] = {10923, 10923, 10923};   /* ~1/3 در Q15 */
        dsp_q15_t state[3+8-1];
        dsp_fir_q15_t fir;
        dsp_q15_t x[8] = {32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767};
        dsp_q15_t y[8];
        CHECK(dsp_fir_init_q15(&fir, h, 3, 8, DSP_FORM_DIRECT, state, 10, 15) == DSP_OK, "fir q15 init");
        dsp_fir_process_block_q15(&fir, x, y, 8);
        CHECK_NEAR(y[2], 32767, 300, "fir q15 y2");
        dsp_fir_reset_q15(&fir);
        for (int i = 0; i < 8; i++) dsp_fir_process_sample_q15(&fir, x[i], &y[i]);
        CHECK_NEAR(y[2], 32767, 300, "fir q15 sample y2");
    }

    /* ---- Biquad Q15: LPF 1000Hz/48k با ورودی 0.25 مقیاس، مقایسه با f32 ---- */
    {
        dsp_biquad_f32_t bq32;
        dsp_biquad_q15_t bq15;
        CHECK(dsp_biquad_init_f32(&bq32, DSP_TDF2) == DSP_OK, "bq15 ref init");
        CHECK(dsp_biquad_set_params_f32(&bq32, DSP_BIQUAD_LPF, 48000, 1000, 0.707f, 0) == DSP_OK, "bq15 ref params");
        CHECK(dsp_biquad_init_q15(&bq15, DSP_DF1) == DSP_OK, "bq15 init");
        CHECK(dsp_biquad_set_params_q15(&bq15, DSP_BIQUAD_LPF, 48000, 1000, 0.707f, 0) == DSP_OK, "bq15 params");
        dsp_f32_t err_max = 0;
        for (int i = 0; i < 2000; i++) {
            dsp_f32_t xf = 0.25f * sinf(2*3.14159265f*300*i/48000.0f);
            dsp_f32_t yf; dsp_q15_t yq;
            dsp_biquad_process_sample_f32(&bq32, xf, &yf);
            dsp_biquad_process_sample_q15(&bq15, (dsp_q15_t)(xf*32767), &yq);
            dsp_f32_t yy = (dsp_f32_t)yq / 32767.0f;
            dsp_f32_t d = fabsf(yf - yy);
            if (d > err_max) err_max = d;
        }
        printf("  [info] biquad q15 vs f32 max err = %g\n", err_max);
        CHECK(err_max < 0.05, "bq15 vs f32 close");
    }

    printf(fails ? "\n*** %d FAILURES ***\n" : "\n*** ALL SMOKE TESTS PASSED ***\n", fails);
    return fails ? 1 : 0;
}
