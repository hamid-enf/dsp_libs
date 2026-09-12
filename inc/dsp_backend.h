/**
 ******************************************************************************
 * @file    dsp_backend.h
 * @brief   رابط Backend — لایه‌ی انتزاع بین API عمومی و پیاده‌سازی
 *
 * دو Backend وجود دارد:
 *   1) Custom DSP Backend   (پیش‌فرض — C بهینه برای Cortex-M7)
 *   2) CMSIS-DSP Backend    (DSP_USE_CMSIS_DSP = 1)
 *
 * انتخاب در زمان کامپایل انجام می‌شود، نه زمان اجرا. دلیل مهندسی:
 *   - در مسیر Sample-by-Sample، dispatch داینامیک (function pointer) هزینه‌ی
 *     indirect call دارد و مانع inlining و Pipeline شدن حلقه‌ها می‌شود.
 *   - حافظه‌ی اضافه (vtable) و پیچیدگی اشکال‌زدایی ندارد.
 *   - امکان تبادل Backend بین دو Build مختلف (مثلاً A/B تست) بدون تغییر
 *     کد کاربر فراهم است.
 *
 * اگر نیاز واقعی به تعویض Backend در زمان اجرا دارید (مثلاً تست A/B روی
 * دستگاه)، می‌توانید دو نمونه با دو Backend بسازید و در سطح Application
 * بین آن‌ها انتخاب کنید — هزینه‌ی آن یک بار per-block است، نه per-sample.
 ******************************************************************************
 */

#ifndef DSP_BACKEND_H
#define DSP_BACKEND_H

#include "dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * شناسایی Backend فعال در این Build
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_BACKEND_CUSTOM = 0,
    DSP_BACKEND_CMSIS  = 1
} dsp_backend_id_t;

static inline dsp_backend_id_t dsp_backend_get_active(void)
{
#if DSP_USE_CMSIS_DSP
    return DSP_BACKEND_CMSIS;
#else
    return DSP_BACKEND_CUSTOM;
#endif
}

static inline const char *dsp_backend_name(void)
{
#if DSP_USE_CMSIS_DSP
    return "CMSIS-DSP";
#else
    return "Custom (Cortex-M7 optimized)";
#endif
}

/* ---------------------------------------------------------------------------
 * پرس‌وجوی پشتیبانی — به برنامه‌های حرفه‌ای اجازه می‌دهد قابلیت‌های
 * Backend فعال را در زمان اجرا بپرسند
 * ------------------------------------------------------------------------- */
typedef struct {
    dsp_backend_id_t id;
    dsp_bool_t supports_fir_direct;
    dsp_bool_t supports_fir_transposed;
    dsp_bool_t supports_fir_symmetric;
    dsp_bool_t supports_sos_tdf2;
    dsp_bool_t supports_f32;
    dsp_bool_t supports_f64;
    dsp_bool_t supports_q15;
    dsp_bool_t supports_q31;
} dsp_backend_info_t;

void dsp_backend_get_info(dsp_backend_info_t *info);

/* ---------------------------------------------------------------------------
 * Cache hooks — پیاده‌سازی ضعیف پیش‌فرض (هیچ‌کار). کاربر برای DMA روی
 * AXI-SRAM باید آن‌ها را بازتعریف کند (مثال‌ها را ببینید).
 * ------------------------------------------------------------------------- */
#if defined(__GNUC__)
  #define DSP_WEAK __attribute__((weak))
#else
  #define DSP_WEAK
#endif

DSP_WEAK void dsp_cache_invalidate_by_addr(const void *addr, uint32_t len)
{
    DSP_UNUSED(addr); DSP_UNUSED(len);
}

DSP_WEAK void dsp_cache_clean_by_addr(const void *addr, uint32_t len)
{
    DSP_UNUSED(addr); DSP_UNUSED(len);
}

#ifdef __cplusplus
}
#endif

#endif /* DSP_BACKEND_H */
