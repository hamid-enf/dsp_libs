/**
 ******************************************************************************
 * @file    bench_main.c — بنچمارک هسته‌ی کتابخانه
 *
 * روی Host:     زمان با clock_gettime (ns) اندازه‌گیری و «سیکل معادل» با فرض
 *               480MHz گزارش می‌شود (برای مقایسه‌ی نسبی).
 * روی STM32H7:  اگر DSP_TARGET_STM32H7 تعریف شود، از DWT->CYCCNT (شمارنده‌ی
 *               سیکل) استفاده می‌شود — دقیق‌ترین روش.
 *
 * معیارهای خروجی: ns/sample یا cycles/sample + حداکثر نرخ نمونه‌برداری نظری
 * در فرکانس CPU داده‌شده (پیش‌فرض 480MHz).
 *
 * کامپایل روی Target:
 *   arm-none-eabi-gcc -DDSP_TARGET_STM32H7 -O3 -mcpu=cortex-m7 -mfloat-abi=hard \
 *       -mfpu=fpv5-d16 -Iinc src/*.c bench/bench_main.c
 ******************************************************************************
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include "dsp.h"

#if defined(DSP_TARGET_STM32H7)
  #include "stm32h7xx.h"
  static uint32_t bench_ticks(void)
  {
      DWT->CYCCNT = 0;
      return DWT->CYCCNT;
  }
  #define BENCH_CLOCK_HZ 480000000u
#else
  #include <time.h>
  static uint64_t bench_ticks(void)
  {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
  }
  #define BENCH_CLOCK_HZ 480000000u   /* فرض برای تبدیل ns به «سیکل معادل» */
#endif

#define BENCH_REPEAT 2000

static double ns_per_call(uint64_t t0, uint64_t t1, unsigned reps)
{
    uint64_t dt = t1 - t0;
    return (double)dt / (double)reps;
}

static void report(const char *name, double ns_per_sample, unsigned samples_per_call)
{
    double ns = ns_per_sample / samples_per_call;
    double cyc = ns * (double)BENCH_CLOCK_HZ / 1e9;
    double maxsr = (cyc > 0) ? (double)BENCH_CLOCK_HZ / cyc : 0;
    printf("%-38s %10.1f ns/sample %10.1f cyc/sample %12.0f maxSR(Hz)\n",
           name, ns, cyc, maxsr);
}

int main(void)
{
    enum { N = 64 };
    static DSP_ALIGN32 dsp_f32_t x[N], y[N];
    static DSP_ALIGN32 dsp_f32_t h[64];
    static DSP_ALIGN32 dsp_f32_t state[64 + N - 1];
    static DSP_ALIGN32 dsp_f32_t sos_coeffs[8 * 5];
    static DSP_ALIGN32 dsp_f32_t sos_state[8 * 2];
    dsp_fir_f32_t fir;
    dsp_biquad_cascade_f32_t cas;
    uint64_t t0, t1;
    unsigned r;
    int i;

    printf("=== STM32H7 DSP Benchmark — %s backend ===\n", dsp_backend_name());
    printf("(Host: زمان واقعی با فرض 480MHz برای «سیکل معادل»)\n\n");
    printf("%-38s %12s %12s %14s\n", "kernel", "ns/sample", "cyc/sample", "maxSR(Hz)");

    for (i = 0; i < N; i++) x[i] = 0.01f * sinf(0.2f * i);
    for (i = 0; i < 64; i++) h[i] = (i < 40) ? 0.025f : 0.0f;
    for (i = 0; i < 8 * 5; i++) sos_coeffs[i] = (i % 5 == 4) ? 0.5f : 0.1f;

    /* --- FIR 64-tap, block 64 --- */
    dsp_fir_init_f32(&fir, h, 64, N, DSP_FORM_DIRECT, state, sizeof(state)/4, 0);
    t0 = bench_ticks();
    for (r = 0; r < BENCH_REPEAT; r++) dsp_fir_process_block_f32(&fir, x, y, N);
    t1 = bench_ticks();
    report("FIR 64-tap (block 64)", ns_per_call(t0, t1, BENCH_REPEAT), N);

    /* --- FIR sample-by-sample --- */
    dsp_fir_reset_f32(&fir);
    t0 = bench_ticks();
    for (r = 0; r < BENCH_REPEAT * N; r++) {
        dsp_f32_t out;
        dsp_fir_process_sample_f32(&fir, x[r % N], &out);
    }
    t1 = bench_ticks();
    report("FIR 64-tap (sample)", ns_per_call(t0, t1, BENCH_REPEAT * N), 1);

    /* --- FIR symmetric 64-tap --- */
    {
        dsp_fir_f32_t fs;
        static DSP_ALIGN32 dsp_f32_t ss[64 + N - 1];
        dsp_fir_init_f32(&fs, h, 64, N, DSP_FORM_SYMMETRIC, ss, sizeof(ss)/4, 0);
        t0 = bench_ticks();
        for (r = 0; r < BENCH_REPEAT; r++) dsp_fir_process_block_f32(&fs, x, y, N);
        t1 = bench_ticks();
        report("FIR 64-tap symmetric", ns_per_call(t0, t1, BENCH_REPEAT), N);
    }

    /* --- Biquad cascade 8-stage TDF2 --- */
    dsp_biquad_cascade_init_f32(&cas, 8, sos_coeffs, sos_state, 16, DSP_TDF2);
    t0 = bench_ticks();
    for (r = 0; r < BENCH_REPEAT; r++) dsp_biquad_cascade_process_block_f32(&cas, x, y, N);
    t1 = bench_ticks();
    report("Biquad cascade 8-st TDF2", ns_per_call(t0, t1, BENCH_REPEAT), N);

#if DSP_USE_Q15
    /* --- FIR q15 64-tap --- */
    {
        static DSP_ALIGN32 dsp_q15_t hq[64], xq[N], yq[N], sq[64 + N - 1];
        dsp_fir_q15_t fq;
        for (i = 0; i < 64; i++) hq[i] = (dsp_q15_t)(h[i] * 32767);
        for (i = 0; i < N; i++) xq[i] = (dsp_q15_t)(x[i] * 32767);
        dsp_fir_init_q15(&fq, hq, 64, N, DSP_FORM_DIRECT, sq, sizeof(sq)/2, 15);
        t0 = bench_ticks();
        for (r = 0; r < BENCH_REPEAT; r++) dsp_fir_process_block_q15(&fq, xq, yq, N);
        t1 = bench_ticks();
        report("FIR q15 64-tap", ns_per_call(t0, t1, BENCH_REPEAT), N);
    }
#endif

    printf("\nنکته: اعداد Host فقط نسبی‌اند؛ روی STM32H7 با DWT دقیق‌ترین نتیجه را بدهید.\n");
    return 0;
}
