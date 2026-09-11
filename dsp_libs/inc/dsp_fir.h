/**
 ******************************************************************************
 * @file    dsp_fir.h
 * @brief   فیلتر FIR — Direct / Transposed / Symmetric
 *
 * دقت‌ها: float32 (پیش‌فرض)، float64، Q15، Q31
 * Backend: داخلی (Cortex-M7 Optimized) یا CMSIS-DSP — به انتخاب کامپایل
 *
 * دو سطح API:
 *   ساده:      dsp_fir_init / dsp_fir_process_sample / dsp_fir_process_block
 *   پیشرفته:   dsp_fir_init_f32 / dsp_fir_update_coeffs_f32 / دسترسی مستقیم
 *              به pCoeffs و pState و head
 *
 * فرمول:  y[n] = sum_{k=0}^{N-1} h[k] * x[n-k]
 *
 * نیازمندی حافظه:
 *   Direct/Symmetric : state = (taps + blockSize - 1) نمونه
 *   Transposed       : state = taps نمونه
 ******************************************************************************
 */

#ifndef DSP_FIR_H
#define DSP_FIR_H

#include "dsp_types.h"

#if DSP_ENABLE_FIR

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * ساختار نمونه — float32
 * =========================================================================== */
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_ARM_F32  arm_fir_instance_f32 arm;   /* نمونه‌ی CMSIS */
#else
  #define DSP_FIR_ARM_F32
#endif

typedef struct {
    uint16_t       taps;        /* تعداد ضرایب */
    uint16_t       blockSize;   /* حداکثر اندازه‌ی بلاک */
    uint8_t        mode;        /* DSP_FORM_DIRECT / DSP_FORM_TRANSPOSED / DSP_FORM_SYMMETRIC */
    const dsp_f32_t *pCoeffs;   /* آرایه‌ی ضرایب h[0..taps-1] (کاربر) */
    dsp_f32_t      *pState;     /* بافر حالت (کاربر) */
    uint16_t       head;        /* ایندکس حلقه برای Sample-by-Sample */
    uint32_t       stateLen;    /* طول بافر حالت */
    DSP_FIR_ARM_F32
} dsp_fir_f32_t;

dsp_err_t dsp_fir_init_f32(dsp_fir_f32_t *p, const dsp_f32_t *coeffs, uint16_t taps,
                           uint16_t blockSize, uint8_t mode,
                           dsp_f32_t *state, uint32_t stateLen, uint8_t shift);
dsp_err_t dsp_fir_process_sample_f32(dsp_fir_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_fir_process_block_f32(dsp_fir_f32_t *p, const dsp_f32_t *src,
                                    dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_fir_update_coeffs_f32(dsp_fir_f32_t *p, const dsp_f32_t *coeffs, uint16_t n);
dsp_err_t dsp_fir_reset_f32(dsp_fir_f32_t *p);

/* ===========================================================================
 * float64
 * =========================================================================== */
typedef struct {
    uint16_t       taps;
    uint16_t       blockSize;
    uint8_t        mode;
    const dsp_f64_t *pCoeffs;
    dsp_f64_t      *pState;
    uint16_t       head;
    uint32_t       stateLen;
} dsp_fir_f64_t;

dsp_err_t dsp_fir_init_f64(dsp_fir_f64_t *p, const dsp_f64_t *coeffs, uint16_t taps,
                           uint16_t blockSize, uint8_t mode,
                           dsp_f64_t *state, uint32_t stateLen, uint8_t shift);
dsp_err_t dsp_fir_process_sample_f64(dsp_fir_f64_t *p, dsp_f64_t x, dsp_f64_t *y);
dsp_err_t dsp_fir_process_block_f64(dsp_fir_f64_t *p, const dsp_f64_t *src,
                                    dsp_f64_t *dst, uint16_t n);
dsp_err_t dsp_fir_update_coeffs_f64(dsp_fir_f64_t *p, const dsp_f64_t *coeffs, uint16_t n);
dsp_err_t dsp_fir_reset_f64(dsp_fir_f64_t *p);

/* ===========================================================================
 * Q15
 * =========================================================================== */
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_ARM_Q15  arm_fir_instance_q15 arm;
#else
  #define DSP_FIR_ARM_Q15
#endif

typedef struct {
    uint16_t       taps;
    uint16_t       blockSize;
    uint8_t        mode;
    const dsp_q15_t *pCoeffs;
    dsp_q15_t      *pState;
    uint16_t       head;
    uint32_t       stateLen;
    uint8_t        shift;   /* شیفت خروجی (معمولاً 15) — قابل تنظیم برای Scaling */
    DSP_FIR_ARM_Q15
} dsp_fir_q15_t;

dsp_err_t dsp_fir_init_q15(dsp_fir_q15_t *p, const dsp_q15_t *coeffs, uint16_t taps,
                           uint16_t blockSize, uint8_t mode,
                           dsp_q15_t *state, uint32_t stateLen, uint8_t shift);
dsp_err_t dsp_fir_process_sample_q15(dsp_fir_q15_t *p, dsp_q15_t x, dsp_q15_t *y);
dsp_err_t dsp_fir_process_block_q15(dsp_fir_q15_t *p, const dsp_q15_t *src,
                                    dsp_q15_t *dst, uint16_t n);
dsp_err_t dsp_fir_update_coeffs_q15(dsp_fir_q15_t *p, const dsp_q15_t *coeffs, uint16_t n);
dsp_err_t dsp_fir_reset_q15(dsp_fir_q15_t *p);

/* ===========================================================================
 * Q31
 * =========================================================================== */
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_ARM_Q31  arm_fir_instance_q31 arm;
#else
  #define DSP_FIR_ARM_Q31
#endif

typedef struct {
    uint16_t       taps;
    uint16_t       blockSize;
    uint8_t        mode;
    const dsp_q31_t *pCoeffs;
    dsp_q31_t      *pState;
    uint16_t       head;
    uint32_t       stateLen;
    uint8_t        shift;
    DSP_FIR_ARM_Q31
} dsp_fir_q31_t;

dsp_err_t dsp_fir_init_q31(dsp_fir_q31_t *p, const dsp_q31_t *coeffs, uint16_t taps,
                           uint16_t blockSize, uint8_t mode,
                           dsp_q31_t *state, uint32_t stateLen, uint8_t shift);
dsp_err_t dsp_fir_process_sample_q31(dsp_fir_q31_t *p, dsp_q31_t x, dsp_q31_t *y);
dsp_err_t dsp_fir_process_block_q31(dsp_fir_q31_t *p, const dsp_q31_t *src,
                                    dsp_q31_t *dst, uint16_t n);
dsp_err_t dsp_fir_update_coeffs_q31(dsp_fir_q31_t *p, const dsp_q31_t *coeffs, uint16_t n);
dsp_err_t dsp_fir_reset_q31(dsp_fir_q31_t *p);

/* ===========================================================================
 * API ساده — با دقت پیش‌فرض (DSP_DEFAULT_PRECISION)
 * =========================================================================== */
#if DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F32
  typedef dsp_fir_f32_t dsp_fir_t;
  #define dsp_fir_init(...)       dsp_fir_init_f32(__VA_ARGS__, 0)
  #define dsp_fir_process_sample  dsp_fir_process_sample_f32
  #define dsp_fir_process_block   dsp_fir_process_block_f32
  #define dsp_fir_update_coeffs   dsp_fir_update_coeffs_f32
  #define dsp_fir_reset           dsp_fir_reset_f32
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F64
  typedef dsp_fir_f64_t dsp_fir_t;
  #define dsp_fir_init(...)       dsp_fir_init_f64(__VA_ARGS__, 0)
  #define dsp_fir_process_sample  dsp_fir_process_sample_f64
  #define dsp_fir_process_block   dsp_fir_process_block_f64
  #define dsp_fir_update_coeffs   dsp_fir_update_coeffs_f64
  #define dsp_fir_reset           dsp_fir_reset_f64
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_Q31
  typedef dsp_fir_q31_t dsp_fir_t;
  #define dsp_fir_init(...)       dsp_fir_init_q31(__VA_ARGS__, 31)
  #define dsp_fir_process_sample  dsp_fir_process_sample_q31
  #define dsp_fir_process_block   dsp_fir_process_block_q31
  #define dsp_fir_update_coeffs   dsp_fir_update_coeffs_q31
  #define dsp_fir_reset           dsp_fir_reset_q31
#else
  typedef dsp_fir_q15_t dsp_fir_t;
  #define dsp_fir_init(...)       dsp_fir_init_q15(__VA_ARGS__, 15)
  #define dsp_fir_process_sample  dsp_fir_process_sample_q15
  #define dsp_fir_process_block   dsp_fir_process_block_q15
  #define dsp_fir_update_coeffs   dsp_fir_update_coeffs_q15
  #define dsp_fir_reset           dsp_fir_reset_q15
#endif

/* ===========================================================================
 * ماکروی تعریف نمونه‌ی استاتیک — بدون هیچ تخصیص داینامیکی
 *   DSP_FIR_INST_F32(myFir, taps, blockSize);
 * ضرایب به‌صورت const در Flash و حالت در RAM (پیش‌فرض) قرار می‌گیرند.
 * =========================================================================== */
#define DSP_FIR_STATE_SIZE(taps, blockSize)  ((uint32_t)((taps) + (blockSize) - 1))

#define DSP_FIR_INST_F32(name, taps_, blockSize_) \
    static dsp_f32_t name##_state[DSP_FIR_STATE_SIZE(taps_, blockSize_)]; \
    static dsp_fir_f32_t name = { (taps_), (blockSize_), 0, NULL, name##_state, 0, \
                                  DSP_FIR_STATE_SIZE(taps_, blockSize_) }

/* ---------------------------------------------------------------------------
 * نام‌های کوتاه (اختیاری)
 * ------------------------------------------------------------------------- */
#if DSP_USE_SHORT_ALIASES
  #define FIR_Init            dsp_fir_init
  #define FIR_ProcessSample   dsp_fir_process_sample
  #define FIR_ProcessBlock    dsp_fir_process_block
  #define FIR_UpdateCoeffs    dsp_fir_update_coeffs
  #define FIR_Reset           dsp_fir_reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* DSP_ENABLE_FIR */
#endif /* DSP_FIR_H */
