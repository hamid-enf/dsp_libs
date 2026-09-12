/**
 ******************************************************************************
 * @file    dsp_iir.h
 * @brief   فیلتر IIR — Direct Form I / II / Transposed DF2 + SOS Cascade
 *
 * معادله‌ی عمومی (مرتبه‌ی N، a0 = 1):
 *   sum_{k=0..N} a_k·y[n-k] = sum_{k=0..M} b_k·x[n-k]
 *
 * هشدار عددی (مهم):
 *   برای مرتبه‌های بالا (N > 4) و به‌ویژه در Fixed-Point، فقط فرم SOS
 *   (Cascade of Second-Order Sections) از نظر پایداری عددی توصیه می‌شود.
 *   فرم‌های مستقیم DF1/DF2 برای مرتبه‌های کوچک (سریع‌تر) در نظر گرفته شده‌اند.
 *
 * چیدمان ضرایب:
 *   SOS  : pCoeffs = [b0 b1 b2 a1 a2] × stages ،  pState = 2 × stages
 *   مستقیم: pCoeffs = [b0..bN | a1..aN]          ،  pState = 2N (DF1) یا N (DF2/TDF2)
 ******************************************************************************
 */

#ifndef DSP_IIR_H
#define DSP_IIR_H

#include "dsp_types.h"

#if DSP_ENABLE_IIR

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * float32
 * =========================================================================== */
typedef struct {
    uint8_t         form;      /* DSP_DF1 / DSP_DF2 / DSP_TDF2 */
    uint16_t        order;     /* مرتبه‌ی فیلتر */
    uint16_t        stages;    /* تعداد طبقات SOS (در حالت SOS) */
    uint8_t         sos;       /* 1 = ضرایب SOS */
    const dsp_f32_t *pCoeffs;
    dsp_f32_t       *pState;
    uint32_t        stateLen;
} dsp_iir_f32_t;

dsp_err_t dsp_iir_init_f32(dsp_iir_f32_t *p, uint8_t form, uint16_t order,
                           const dsp_f32_t *coeffs, dsp_f32_t *state,
                           uint32_t stateLen, uint8_t sos);
dsp_err_t dsp_iir_process_sample_f32(dsp_iir_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_iir_process_block_f32(dsp_iir_f32_t *p, const dsp_f32_t *src,
                                    dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_iir_update_coeffs_f32(dsp_iir_f32_t *p, const dsp_f32_t *coeffs);
dsp_err_t dsp_iir_reset_f32(dsp_iir_f32_t *p);

/* ===========================================================================
 * float64
 * =========================================================================== */
typedef struct {
    uint8_t         form;
    uint16_t        order;
    uint16_t        stages;
    uint8_t         sos;
    const dsp_f64_t *pCoeffs;
    dsp_f64_t       *pState;
    uint32_t        stateLen;
} dsp_iir_f64_t;

dsp_err_t dsp_iir_init_f64(dsp_iir_f64_t *p, uint8_t form, uint16_t order,
                           const dsp_f64_t *coeffs, dsp_f64_t *state,
                           uint32_t stateLen, uint8_t sos);
dsp_err_t dsp_iir_process_sample_f64(dsp_iir_f64_t *p, dsp_f64_t x, dsp_f64_t *y);
dsp_err_t dsp_iir_process_block_f64(dsp_iir_f64_t *p, const dsp_f64_t *src,
                                    dsp_f64_t *dst, uint16_t n);
dsp_err_t dsp_iir_update_coeffs_f64(dsp_iir_f64_t *p, const dsp_f64_t *coeffs);
dsp_err_t dsp_iir_reset_f64(dsp_iir_f64_t *p);

/* ===========================================================================
 * Q15 — تنها فرم SOS (پایداری عددی)؛ مستقیم فقط تا مرتبه‌ی 2 پشتیبانی می‌شود
 * =========================================================================== */
typedef struct {
    uint8_t         form;
    uint16_t        order;
    uint16_t        stages;
    uint8_t         sos;
    const dsp_q15_t *pCoeffs;
    uint8_t         shift;
    dsp_q15_t       *pState;
    uint32_t        stateLen;
} dsp_iir_q15_t;

dsp_err_t dsp_iir_init_q15(dsp_iir_q15_t *p, uint8_t form, uint16_t order,
                           const dsp_q15_t *coeffs, uint8_t shift,
                           dsp_q15_t *state, uint32_t stateLen, uint8_t sos);
dsp_err_t dsp_iir_process_sample_q15(dsp_iir_q15_t *p, dsp_q15_t x, dsp_q15_t *y);
dsp_err_t dsp_iir_process_block_q15(dsp_iir_q15_t *p, const dsp_q15_t *src,
                                    dsp_q15_t *dst, uint16_t n);
dsp_err_t dsp_iir_reset_q15(dsp_iir_q15_t *p);

/* ===========================================================================
 * Q31 — مانند Q15
 * =========================================================================== */
typedef struct {
    uint8_t         form;
    uint16_t        order;
    uint16_t        stages;
    uint8_t         sos;
    const dsp_q31_t *pCoeffs;
    uint8_t         shift;
    dsp_q31_t       *pState;
    uint32_t        stateLen;
} dsp_iir_q31_t;

dsp_err_t dsp_iir_init_q31(dsp_iir_q31_t *p, uint8_t form, uint16_t order,
                           const dsp_q31_t *coeffs, uint8_t shift,
                           dsp_q31_t *state, uint32_t stateLen, uint8_t sos);
dsp_err_t dsp_iir_process_sample_q31(dsp_iir_q31_t *p, dsp_q31_t x, dsp_q31_t *y);
dsp_err_t dsp_iir_process_block_q31(dsp_iir_q31_t *p, const dsp_q31_t *src,
                                    dsp_q31_t *dst, uint16_t n);
dsp_err_t dsp_iir_reset_q31(dsp_iir_q31_t *p);

/* ===========================================================================
 * API ساده
 * =========================================================================== */
#if DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F32
  typedef dsp_iir_f32_t dsp_iir_t;
  #define dsp_iir_init            dsp_iir_init_f32
  #define dsp_iir_process_sample  dsp_iir_process_sample_f32
  #define dsp_iir_process_block   dsp_iir_process_block_f32
  #define dsp_iir_update_coeffs   dsp_iir_update_coeffs_f32
  #define dsp_iir_reset           dsp_iir_reset_f32
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F64
  typedef dsp_iir_f64_t dsp_iir_t;
  #define dsp_iir_init            dsp_iir_init_f64
  #define dsp_iir_process_sample  dsp_iir_process_sample_f64
  #define dsp_iir_process_block   dsp_iir_process_block_f64
  #define dsp_iir_update_coeffs   dsp_iir_update_coeffs_f64
  #define dsp_iir_reset           dsp_iir_reset_f64
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_Q31
  typedef dsp_iir_q31_t dsp_iir_t;
  #define dsp_iir_init            dsp_iir_init_q31
  #define dsp_iir_process_sample  dsp_iir_process_sample_q31
  #define dsp_iir_process_block   dsp_iir_process_block_q31
  #define dsp_iir_reset           dsp_iir_reset_q31
#else
  typedef dsp_iir_q15_t dsp_iir_t;
  #define dsp_iir_init            dsp_iir_init_q15
  #define dsp_iir_process_sample  dsp_iir_process_sample_q15
  #define dsp_iir_process_block   dsp_iir_process_block_q15
  #define dsp_iir_reset           dsp_iir_reset_q15
#endif

#if DSP_USE_SHORT_ALIASES
  #define IIR_Init          dsp_iir_init
  #define IIR_ProcessSample dsp_iir_process_sample
  #define IIR_ProcessBlock  dsp_iir_process_block
  #define IIR_Reset         dsp_iir_reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* DSP_ENABLE_IIR */
#endif /* DSP_IIR_H */
