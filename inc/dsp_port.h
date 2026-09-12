/**
 ******************************************************************************
 * @file    dsp_port.h
 * @brief   لایه‌ی Portability — تنها نقطه‌ی وابستگی به سخت‌افزار/کامپایلر
 *
 * این فایل تعیین می‌کند:
 *   - انواع پایه (float32_t و ...) از کجا بیایند (CMSIS یا تعریف داخلی)
 *   - توابع ریاضی و اشباع (saturation)
 *   - ماکروهای چیدمان حافظه (DTCM / SRAM / AXI-SRAM) برای Cortex-M7
 *
 * روی STM32H7 با CMSIS، انواع از arm_math.h می‌آیند؛ بدون CMSIS (یا روی
 * Host برای تست)، انواع معادل به‌صورت داخلی تعریف می‌شوند. هیچ‌جای دیگر
 * کتابخانه مستقیماً به CMSIS وابسته نیست.
 ******************************************************************************
 */

#ifndef DSP_PORT_H
#define DSP_PORT_H

#include "dsp_config.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

/* ---------------------------------------------------------------------------
 * CMSIS: اگر فعال باشد، انواع arm_math در دسترس هستند (فقط برای Backend)
 * ------------------------------------------------------------------------- */
#if DSP_USE_CMSIS_DSP
  #include "arm_math.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * انواع پایه‌ی کتابخانه
 * ------------------------------------------------------------------------- */
typedef float          dsp_f32_t;   /* single precision */
typedef double         dsp_f64_t;   /* double precision  */
typedef int16_t        dsp_q15_t;   /* Q1.15 fixed point */
typedef int32_t        dsp_q31_t;   /* Q1.31 fixed point */
typedef int64_t        dsp_q63_t;   /* 64-bit accumulator */
typedef uint32_t       dsp_uint_t;
typedef uint8_t        dsp_bool_t;

#define DSP_TRUE  1u
#define DSP_FALSE 0u

/* ---------------------------------------------------------------------------
 * ثابت‌های عددی
 * ------------------------------------------------------------------------- */
#define DSP_PI        3.14159265358979323846
#define DSP_PI_F      3.14159265358979323846f
#define DSP_TWO_PI    6.28318530717958647692
#define DSP_TWO_PI_F  6.28318530717958647692f
#define DSP_SQRT2     1.41421356237309504880

#define DSP_Q15_MAX   32767
#define DSP_Q15_MIN   (-32768)
#define DSP_Q31_MAX   2147483647LL
#define DSP_Q31_MIN   (-2147483648LL)
#define DSP_Q63_MAX_POS  9223372036854775807LL
#define DSP_Q63_MIN_NEG  (-9223372036854775807LL - 1)

/* ---------------------------------------------------------------------------
 * حداقل/حداکثر و ابزارهای کوچک
 * ------------------------------------------------------------------------- */
static inline dsp_f32_t dsp_min_f32(dsp_f32_t a, dsp_f32_t b) { return (a < b) ? a : b; }
static inline dsp_f32_t dsp_max_f32(dsp_f32_t a, dsp_f32_t b) { return (a > b) ? a : b; }
static inline dsp_f64_t dsp_min_f64(dsp_f64_t a, dsp_f64_t b) { return (a < b) ? a : b; }
static inline dsp_f64_t dsp_max_f64(dsp_f64_t a, dsp_f64_t b) { return (a > b) ? a : b; }
static inline dsp_f32_t dsp_clamp_f32(dsp_f32_t x, dsp_f32_t lo, dsp_f32_t hi)
{
    return (x < lo) ? lo : ((x > hi) ? hi : x);
}

#define DSP_UNUSED(x) ((void)(x))

/* ---------------------------------------------------------------------------
 * اشباع (Saturation) — در Fixed-Point ضروری است
 * ---------------------------------------------------------------------------
 * روی Cortex-M7 با CMSIS از دستور یک‌سیکلی __SSAT استفاده می‌شود؛ در غیر این
 * صورت پیاده‌سازی قابل‌حمل C. پیاده‌سازی‌ها رفتار یکسانی دارند.
 */
#if DSP_USE_CMSIS_DSP
  #define DSP_Q15_SAT(x)   ((dsp_q15_t)__SSAT((x), 16))
  #define DSP_Q31_SAT(x)   ((dsp_q31_t)__SSAT((x), 32))
#else
static inline dsp_q15_t dsp_q15_sat(dsp_q31_t x)
{
    return (dsp_q15_t)((x >  DSP_Q15_MAX) ?  DSP_Q15_MAX :
                       (x <  DSP_Q15_MIN) ?  DSP_Q15_MIN : x);
}
static inline dsp_q31_t dsp_q31_sat(dsp_q63_t x)
{
    return (dsp_q31_t)((x >  DSP_Q31_MAX) ?  DSP_Q31_MAX :
                       (x <  DSP_Q31_MIN) ?  DSP_Q31_MIN : x);
}
  #define DSP_Q15_SAT(x)   dsp_q15_sat((dsp_q31_t)(x))
  #define DSP_Q31_SAT(x)   dsp_q31_sat((dsp_q63_t)(x))
#endif

/* گردکردن و شیفت برای Fixed-Point: acc >> shift با گردکردن متقارن */
static inline dsp_q15_t dsp_shift_round_q15(dsp_q31_t acc, int32_t shift)
{
    if (shift > 0) {
        dsp_q31_t r = (acc + (1 << (shift - 1)));
        return DSP_Q15_SAT(r >> shift);
    }
    return DSP_Q15_SAT(acc << (-shift));
}
static inline dsp_q31_t dsp_shift_round_q31(dsp_q63_t acc, int32_t shift)
{
    if (shift > 0) {
        dsp_q63_t r = (acc + ((dsp_q63_t)1 << (shift - 1)));
        return DSP_Q31_SAT(r >> shift);
    }
    return DSP_Q31_SAT(acc << (-shift));
}

/* جمع اشباعی (Saturating Add) — برای محافظت از سرریز accumulator در Fixed-Point */
static inline dsp_q31_t dsp_q31_sat_add(dsp_q31_t a, dsp_q31_t b)
{
    dsp_q63_t s = (dsp_q63_t)a + (dsp_q63_t)b;
    return DSP_Q31_SAT(s);
}
static inline dsp_q63_t dsp_q63_sat_add(dsp_q63_t a, dsp_q63_t b)
{
    dsp_q63_t s = a + b;
    if ((a > 0) && (b > 0) && (s < 0)) return DSP_Q63_MAX_POS;
    if ((a < 0) && (b < 0) && (s >= 0)) return DSP_Q63_MIN_NEG;
    return s;
}

/* ---------------------------------------------------------------------------
 * توابع ریاضی (در صورت نیاز به نسخه‌های بهینه برای M7 می‌توان آن‌ها را با
 * توابع CMSIS (arm_sin_f32 و ...) جایگزین کرد — همین‌جا، فقط همین‌جا)
 * ------------------------------------------------------------------------- */
static inline dsp_f32_t dsp_sin_f32(dsp_f32_t x) { return sinf(x); }
static inline dsp_f32_t dsp_cos_f32(dsp_f32_t x) { return cosf(x); }
static inline dsp_f32_t dsp_tan_f32(dsp_f32_t x) { return tanf(x); }
static inline dsp_f32_t dsp_sqrt_f32(dsp_f32_t x) { return sqrtf(x); }
static inline dsp_f32_t dsp_atan2_f32(dsp_f32_t y, dsp_f32_t x) { return atan2f(y, x); }
static inline dsp_f32_t dsp_pow_f32(dsp_f32_t b, dsp_f32_t e) { return powf(b, e); }
static inline dsp_f32_t dsp_exp_f32(dsp_f32_t x) { return expf(x); }
static inline dsp_f32_t dsp_log10_f32(dsp_f32_t x) { return log10f(x); }
static inline dsp_f32_t dsp_log_f32(dsp_f32_t x) { return logf(x); }
static inline dsp_f32_t dsp_abs_f32(dsp_f32_t x) { return fabsf(x); }
static inline dsp_f32_t dsp_floor_f32(dsp_f32_t x) { return floorf(x); }
static inline dsp_f32_t dsp_ceil_f32(dsp_f32_t x) { return ceilf(x); }
static inline dsp_f32_t dsp_fmod_f32(dsp_f32_t a, dsp_f32_t b) { return fmodf(a, b); }
static inline dsp_f32_t dsp_asinh_f32(dsp_f32_t x) { return asinhf(x); }
static inline dsp_f32_t dsp_sinh_f32(dsp_f32_t x) { return sinhf(x); }
static inline dsp_f32_t dsp_cosh_f32(dsp_f32_t x) { return coshf(x); }

/* ---------------------------------------------------------------------------
 * چیدمان حافظه — Cortex-M7 / STM32H7
 * ---------------------------------------------------------------------------
 * - DTCM:     سریع‌ترین برای CPU اما DMA به آن دسترسی ندارد! (بافرهای
 *             پردازش CPU-only را اینجا بگذارید)
 * - SRAM1..3: قابل دسترسی CPU و DMA (بافرهای DMA را اینجا بگذارید)
 * - AXI-SRAM: بزرگ، قابل دسترسی DMA، کش‌دار (نیازمند Cache Maintenance)
 *
 * روی Host (تست) این ماکروها خالی‌اند. برای GCC-ARM معتبرند.
 * برای IAR/Keil، معادل pragma را اضافه کنید (در مستندات Architecture).
 */
#if defined(__GNUC__) && (defined(__arm__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__))
  #define DSP_ALIGN16        __attribute__((aligned(16)))
  #define DSP_ALIGN32        __attribute__((aligned(32)))
  #define DSP_PLACE_DTCM     __attribute__((section(".dtcmram")))
  #define DSP_PLACE_SRAM1    __attribute__((section(".sram1")))
  #define DSP_PLACE_SRAM2    __attribute__((section(".sram2")))
  #define DSP_PLACE_AXISRAM  __attribute__((section(".axisram")))
  #define DSP_PLACE_ITCM     __attribute__((section(".itcmram")))
#else
  #define DSP_ALIGN16
  #define DSP_ALIGN32
  #define DSP_PLACE_DTCM
  #define DSP_PLACE_SRAM1
  #define DSP_PLACE_SRAM2
  #define DSP_PLACE_AXISRAM
  #define DSP_PLACE_ITCM
#endif

/* ---------------------------------------------------------------------------
 * Cache — hooks قابل پیاده‌سازی توسط کاربر (برای DMA روی AXI-SRAM)
 * اگر کاربر توابع dsp_cache_invalidate/clean را تعریف نکند، نسخه‌ی ضعیف
 * (هیچ‌کار) استفاده می‌شود. روی STM32H7 این‌ها را به SCB_InvalidateDCache و
 * SCB_CleanDCache نگاشت کنید (مثال‌ها را ببینید).
 * ------------------------------------------------------------------------- */
void dsp_cache_invalidate_by_addr(const void *addr, uint32_t len);
void dsp_cache_clean_by_addr(const void *addr, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* DSP_PORT_H */
