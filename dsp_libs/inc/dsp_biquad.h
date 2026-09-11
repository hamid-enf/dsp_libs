/**
 ******************************************************************************
 * @file    dsp_biquad.h
 * @brief   موتور Biquad حرفه‌ای — Single + Cascade (SOS)
 *
 * ساختارها:      Direct Form I (DF1) / Direct Form II (DF2) / Transposed DF2
 * انواع فیلتر:   LPF / HPF / BPF / BSF / Notch / Peaking EQ / Low-Shelf / High-Shelf
 *
 * معادله‌ی Biquad (فرم نرمال‌شده با a0 = 1):
 *   y[n] = b0·x[n] + b1·x[n-1] + b2·x[n-2] − a1·y[n-1] − a2·y[n-2]
 *
 * API پیشرفته امکان مشاهده/تغییر مستقیم ضرایب (b0..a2) و بازتولید ضرایب از
 * پارامترها (Fs/Fc/Q/Gain) را می‌دهد.
 *
 * Fixed-Point (Q15/Q31):
 *   ضرایب با یک shift خودکار (برای حداکثر استفاده از بیت‌ها) ذخیره می‌شوند.
 *   فیلد shift پس از set_params/init تنظیم می‌شود و برای خروجی اعمال می‌گردد.
 ******************************************************************************
 */

#ifndef DSP_BIQUAD_H
#define DSP_BIQUAD_H

#include "dsp_types.h"

#if DSP_ENABLE_BIQUAD

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * دقت float32
 * =========================================================================== */
typedef struct {
    dsp_f32_t  b0, b1, b2;      /* ضرایب پیش‌خور */
    dsp_f32_t  a1, a2;          /* ضرایب بازخورد (a0=1) */
    dsp_f32_t  x1, x2, y1, y2;  /* حالت DF1 */
    dsp_f32_t  d1, d2;          /* حالت DF2T / DF2 */
    uint8_t    form;            /* DSP_DF1 / DSP_DF2 / DSP_TDF2 */
    /* پارامترهای طراحی (برای بازتولید ضرایب) */
    uint8_t    type;            /* dsp_biquad_type_t */
    uint32_t   fs;
    dsp_f32_t  fc;
    dsp_f32_t  q;
    dsp_f32_t  gain_db;
} dsp_biquad_f32_t;

dsp_err_t dsp_biquad_init_f32(dsp_biquad_f32_t *p, uint8_t form);
dsp_err_t dsp_biquad_set_coeffs_f32(dsp_biquad_f32_t *p, dsp_f32_t b0, dsp_f32_t b1,
                                    dsp_f32_t b2, dsp_f32_t a1, dsp_f32_t a2);
dsp_err_t dsp_biquad_set_coeffs_arr_f32(dsp_biquad_f32_t *p, const dsp_f32_t c[5]);
dsp_err_t dsp_biquad_get_coeffs_f32(const dsp_biquad_f32_t *p, dsp_f32_t c[5]);
dsp_err_t dsp_biquad_set_params_f32(dsp_biquad_f32_t *p, uint8_t type,
                                    uint32_t fs, dsp_f32_t fc, dsp_f32_t q, dsp_f32_t gain_db);
dsp_err_t dsp_biquad_process_sample_f32(dsp_biquad_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_biquad_process_block_f32(dsp_biquad_f32_t *p, const dsp_f32_t *src,
                                       dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_biquad_reset_f32(dsp_biquad_f32_t *p);

/* Cascade (SOS) — float32 */
typedef struct {
    uint16_t        stages;    /* تعداد طبقات */
    uint8_t         form;      /* DSP_TDF2 پیشنهادی */
    const dsp_f32_t *pCoeffs;  /* 5 ضریب برای هر طبقه: b0 b1 b2 a1 a2 */
    dsp_f32_t       *pState;   /* 2 حالت برای هر طبقه (d1,d2 در TDF2) */
    uint32_t        stateLen;
} dsp_biquad_cascade_f32_t;

dsp_err_t dsp_biquad_cascade_init_f32(dsp_biquad_cascade_f32_t *p, uint16_t stages,
                                      const dsp_f32_t *coeffs, dsp_f32_t *state,
                                      uint32_t stateLen, uint8_t form);
dsp_err_t dsp_biquad_cascade_process_sample_f32(dsp_biquad_cascade_f32_t *p,
                                                dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_biquad_cascade_process_block_f32(dsp_biquad_cascade_f32_t *p,
                                               const dsp_f32_t *src, dsp_f32_t *dst,
                                               uint16_t n);
dsp_err_t dsp_biquad_cascade_update_coeffs_f32(dsp_biquad_cascade_f32_t *p,
                                               const dsp_f32_t *coeffs, uint16_t stages);
dsp_err_t dsp_biquad_cascade_reset_f32(dsp_biquad_cascade_f32_t *p);

/* ===========================================================================
 * دقت float64
 * =========================================================================== */
typedef struct {
    dsp_f64_t  b0, b1, b2;
    dsp_f64_t  a1, a2;
    dsp_f64_t  x1, x2, y1, y2;
    dsp_f64_t  d1, d2;
    uint8_t    form;
    uint8_t    type;
    uint32_t   fs;
    dsp_f64_t  fc;
    dsp_f64_t  q;
    dsp_f64_t  gain_db;
} dsp_biquad_f64_t;

dsp_err_t dsp_biquad_init_f64(dsp_biquad_f64_t *p, uint8_t form);
dsp_err_t dsp_biquad_set_coeffs_f64(dsp_biquad_f64_t *p, dsp_f64_t b0, dsp_f64_t b1,
                                    dsp_f64_t b2, dsp_f64_t a1, dsp_f64_t a2);
dsp_err_t dsp_biquad_set_params_f64(dsp_biquad_f64_t *p, uint8_t type,
                                    uint32_t fs, dsp_f64_t fc, dsp_f64_t q, dsp_f64_t gain_db);
dsp_err_t dsp_biquad_get_coeffs_f64(const dsp_biquad_f64_t *p, dsp_f64_t c[5]);
dsp_err_t dsp_biquad_process_sample_f64(dsp_biquad_f64_t *p, dsp_f64_t x, dsp_f64_t *y);
dsp_err_t dsp_biquad_process_block_f64(dsp_biquad_f64_t *p, const dsp_f64_t *src,
                                       dsp_f64_t *dst, uint16_t n);
dsp_err_t dsp_biquad_reset_f64(dsp_biquad_f64_t *p);

typedef struct {
    uint16_t        stages;
    uint8_t         form;
    const dsp_f64_t *pCoeffs;
    dsp_f64_t       *pState;
    uint32_t        stateLen;
} dsp_biquad_cascade_f64_t;

dsp_err_t dsp_biquad_cascade_init_f64(dsp_biquad_cascade_f64_t *p, uint16_t stages,
                                      const dsp_f64_t *coeffs, dsp_f64_t *state,
                                      uint32_t stateLen, uint8_t form);
dsp_err_t dsp_biquad_cascade_process_sample_f64(dsp_biquad_cascade_f64_t *p,
                                                dsp_f64_t x, dsp_f64_t *y);
dsp_err_t dsp_biquad_cascade_process_block_f64(dsp_biquad_cascade_f64_t *p,
                                               const dsp_f64_t *src, dsp_f64_t *dst,
                                               uint16_t n);
dsp_err_t dsp_biquad_cascade_reset_f64(dsp_biquad_cascade_f64_t *p);

/* ===========================================================================
 * دقت Q15
 * =========================================================================== */
typedef struct {
    dsp_q15_t  b0, b1, b2;
    dsp_q15_t  a1, a2;
    dsp_q15_t  x1, x2, y1, y2;   /* DF1 */
    dsp_q15_t  d1, d2;           /* DF2T/DF2 */
    uint8_t    shift;            /* شیفت خروجی = 15 + s */
    uint8_t    form;
    uint8_t    type;
    uint32_t   fs;
    dsp_f32_t  fc, q, gain_db;
} dsp_biquad_q15_t;

dsp_err_t dsp_biquad_init_q15(dsp_biquad_q15_t *p, uint8_t form);
dsp_err_t dsp_biquad_set_coeffs_q15(dsp_biquad_q15_t *p, dsp_q15_t b0, dsp_q15_t b1,
                                    dsp_q15_t b2, dsp_q15_t a1, dsp_q15_t a2, uint8_t shift);
dsp_err_t dsp_biquad_set_params_q15(dsp_biquad_q15_t *p, uint8_t type,
                                    uint32_t fs, dsp_f32_t fc, dsp_f32_t q, dsp_f32_t gain_db);
dsp_err_t dsp_biquad_get_coeffs_q15(const dsp_biquad_q15_t *p, dsp_q15_t c[5], uint8_t *shift);
dsp_err_t dsp_biquad_process_sample_q15(dsp_biquad_q15_t *p, dsp_q15_t x, dsp_q15_t *y);
dsp_err_t dsp_biquad_process_block_q15(dsp_biquad_q15_t *p, const dsp_q15_t *src,
                                       dsp_q15_t *dst, uint16_t n);
dsp_err_t dsp_biquad_reset_q15(dsp_biquad_q15_t *p);

typedef struct {
    uint16_t        stages;
    uint8_t         form;
    const dsp_q15_t *pCoeffs;    /* 5*stages */
    uint8_t         shift;
    dsp_q15_t       *pState;
    uint32_t        stateLen;
} dsp_biquad_cascade_q15_t;

dsp_err_t dsp_biquad_cascade_init_q15(dsp_biquad_cascade_q15_t *p, uint16_t stages,
                                      const dsp_q15_t *coeffs, uint8_t shift,
                                      dsp_q15_t *state, uint32_t stateLen, uint8_t form);
dsp_err_t dsp_biquad_cascade_process_sample_q15(dsp_biquad_cascade_q15_t *p,
                                                dsp_q15_t x, dsp_q15_t *y);
dsp_err_t dsp_biquad_cascade_process_block_q15(dsp_biquad_cascade_q15_t *p,
                                               const dsp_q15_t *src, dsp_q15_t *dst,
                                               uint16_t n);
dsp_err_t dsp_biquad_cascade_reset_q15(dsp_biquad_cascade_q15_t *p);

/* ===========================================================================
 * دقت Q31
 * =========================================================================== */
typedef struct {
    dsp_q31_t  b0, b1, b2;
    dsp_q31_t  a1, a2;
    dsp_q31_t  x1, x2, y1, y2;
    dsp_q31_t  d1, d2;
    uint8_t    shift;
    uint8_t    form;
    uint8_t    type;
    uint32_t   fs;
    dsp_f32_t  fc, q, gain_db;
} dsp_biquad_q31_t;

dsp_err_t dsp_biquad_init_q31(dsp_biquad_q31_t *p, uint8_t form);
dsp_err_t dsp_biquad_set_coeffs_q31(dsp_biquad_q31_t *p, dsp_q31_t b0, dsp_q31_t b1,
                                    dsp_q31_t b2, dsp_q31_t a1, dsp_q31_t a2, uint8_t shift);
dsp_err_t dsp_biquad_set_params_q31(dsp_biquad_q31_t *p, uint8_t type,
                                    uint32_t fs, dsp_f32_t fc, dsp_f32_t q, dsp_f32_t gain_db);
dsp_err_t dsp_biquad_get_coeffs_q31(const dsp_biquad_q31_t *p, dsp_q31_t c[5], uint8_t *shift);
dsp_err_t dsp_biquad_process_sample_q31(dsp_biquad_q31_t *p, dsp_q31_t x, dsp_q31_t *y);
dsp_err_t dsp_biquad_process_block_q31(dsp_biquad_q31_t *p, const dsp_q31_t *src,
                                       dsp_q31_t *dst, uint16_t n);
dsp_err_t dsp_biquad_reset_q31(dsp_biquad_q31_t *p);

typedef struct {
    uint16_t        stages;
    uint8_t         form;
    const dsp_q31_t *pCoeffs;
    uint8_t         shift;
    dsp_q31_t       *pState;
    uint32_t        stateLen;
} dsp_biquad_cascade_q31_t;

dsp_err_t dsp_biquad_cascade_init_q31(dsp_biquad_cascade_q31_t *p, uint16_t stages,
                                      const dsp_q31_t *coeffs, uint8_t shift,
                                      dsp_q31_t *state, uint32_t stateLen, uint8_t form);
dsp_err_t dsp_biquad_cascade_process_sample_q31(dsp_biquad_cascade_q31_t *p,
                                                dsp_q31_t x, dsp_q31_t *y);
dsp_err_t dsp_biquad_cascade_process_block_q31(dsp_biquad_cascade_q31_t *p,
                                               const dsp_q31_t *src, dsp_q31_t *dst,
                                               uint16_t n);
dsp_err_t dsp_biquad_cascade_reset_q31(dsp_biquad_cascade_q31_t *p);

/* ===========================================================================
 * API ساده — دقت پیش‌فرض
 * =========================================================================== */
#if DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F32
  typedef dsp_biquad_f32_t dsp_biquad_t;
  typedef dsp_biquad_cascade_f32_t dsp_biquad_cascade_t;
  #define dsp_biquad_init            dsp_biquad_init_f32
  #define dsp_biquad_set_params       dsp_biquad_set_params_f32
  #define dsp_biquad_set_coeffs       dsp_biquad_set_coeffs_f32
  #define dsp_biquad_process_sample   dsp_biquad_process_sample_f32
  #define dsp_biquad_process_block    dsp_biquad_process_block_f32
  #define dsp_biquad_reset            dsp_biquad_reset_f32
  #define dsp_biquad_cascade_init     dsp_biquad_cascade_init_f32
  #define dsp_biquad_cascade_process_sample dsp_biquad_cascade_process_sample_f32
  #define dsp_biquad_cascade_process_block  dsp_biquad_cascade_process_block_f32
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_F64
  typedef dsp_biquad_f64_t dsp_biquad_t;
  typedef dsp_biquad_cascade_f64_t dsp_biquad_cascade_t;
  #define dsp_biquad_init            dsp_biquad_init_f64
  #define dsp_biquad_set_params       dsp_biquad_set_params_f64
  #define dsp_biquad_set_coeffs       dsp_biquad_set_coeffs_f64
  #define dsp_biquad_process_sample   dsp_biquad_process_sample_f64
  #define dsp_biquad_process_block    dsp_biquad_process_block_f64
  #define dsp_biquad_reset            dsp_biquad_reset_f64
  #define dsp_biquad_cascade_init     dsp_biquad_cascade_init_f64
  #define dsp_biquad_cascade_process_sample dsp_biquad_cascade_process_sample_f64
  #define dsp_biquad_cascade_process_block  dsp_biquad_cascade_process_block_f64
#elif DSP_DEFAULT_PRECISION == DSP_PRECISION_ID_Q31
  typedef dsp_biquad_q31_t dsp_biquad_t;
  typedef dsp_biquad_cascade_q31_t dsp_biquad_cascade_t;
  #define dsp_biquad_init            dsp_biquad_init_q31
  #define dsp_biquad_set_params       dsp_biquad_set_params_q31
  #define dsp_biquad_set_coeffs       dsp_biquad_set_coeffs_q31
  #define dsp_biquad_process_sample   dsp_biquad_process_sample_q31
  #define dsp_biquad_process_block    dsp_biquad_process_block_q31
  #define dsp_biquad_reset            dsp_biquad_reset_q31
  #define dsp_biquad_cascade_init     dsp_biquad_cascade_init_q31
  #define dsp_biquad_cascade_process_sample dsp_biquad_cascade_process_sample_q31
  #define dsp_biquad_cascade_process_block  dsp_biquad_cascade_process_block_q31
#else
  typedef dsp_biquad_q15_t dsp_biquad_t;
  typedef dsp_biquad_cascade_q15_t dsp_biquad_cascade_t;
  #define dsp_biquad_init            dsp_biquad_init_q15
  #define dsp_biquad_set_params       dsp_biquad_set_params_q15
  #define dsp_biquad_set_coeffs       dsp_biquad_set_coeffs_q15
  #define dsp_biquad_process_sample   dsp_biquad_process_sample_q15
  #define dsp_biquad_process_block    dsp_biquad_process_block_q15
  #define dsp_biquad_reset            dsp_biquad_reset_q15
  #define dsp_biquad_cascade_init     dsp_biquad_cascade_init_q15
  #define dsp_biquad_cascade_process_sample dsp_biquad_cascade_process_sample_q15
  #define dsp_biquad_cascade_process_block  dsp_biquad_cascade_process_block_q15
#endif

/* ---------------------------------------------------------------------------
 * نام‌های کوتاه (اختیاری)
 * ------------------------------------------------------------------------- */
#if DSP_USE_SHORT_ALIASES
  #define Biquad_Init          dsp_biquad_init
  #define Biquad_SetParams     dsp_biquad_set_params
  #define Biquad_SetCoeffs     dsp_biquad_set_coeffs
  #define Biquad_ProcessSample dsp_biquad_process_sample
  #define Biquad_ProcessBlock  dsp_biquad_process_block
  #define Biquad_Reset         dsp_biquad_reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* DSP_ENABLE_BIQUAD */
#endif /* DSP_BIQUAD_H */
