/**
 * @file dsp_moving.h — فیلترهای میانگین متحرک: SMA / WMA / EMA
 *
 * SMA:  y[n] = (1/W)·Σ_{k=0}^{W-1} x[n-k]          — O(1) با Running Sum
 * WMA:  y[n] = Σ_{k=0}^{W-1} w[k]·x[n-k] / Σ w      — ضرایب دلخواه
 * EMA:  y[n] = α·x[n] + (1-α)·y[n-1]               — O(1)
 */
#ifndef DSP_MOVING_H
#define DSP_MOVING_H
#include "dsp_types.h"
#if DSP_ENABLE_MOVING_AVERAGE
#ifdef __cplusplus
extern "C" {
#endif

/* Simple Moving Average — O(1) per sample */
typedef struct {
    uint16_t   window;
    dsp_f32_t *pBuf;      /* window نمونه */
    dsp_f32_t  sum;
    uint16_t   idx;
    uint16_t   count;     /* تعداد نمونه‌های جمع‌شده (warm-up) */
} dsp_sma_f32_t;

dsp_err_t dsp_sma_init_f32(dsp_sma_f32_t *p, uint16_t window, dsp_f32_t *buf);
dsp_err_t dsp_sma_process_f32(dsp_sma_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_sma_process_block_f32(dsp_sma_f32_t *p, const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_sma_reset_f32(dsp_sma_f32_t *p);

/* Weighted Moving Average */
typedef struct {
    uint16_t     window;
    const dsp_f32_t *pW;   /* ضرایب w[0..W-1] */
    dsp_f32_t   *pBuf;
    uint16_t     idx;
    dsp_f32_t    wsum;
} dsp_wma_f32_t;

dsp_err_t dsp_wma_init_f32(dsp_wma_f32_t *p, uint16_t window, const dsp_f32_t *weights, dsp_f32_t *buf);
dsp_err_t dsp_wma_process_f32(dsp_wma_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_wma_reset_f32(dsp_wma_f32_t *p);

/* Exponential Moving Average */
typedef struct {
    dsp_f32_t alpha;
    dsp_f32_t y;
} dsp_ema_f32_t;

dsp_err_t dsp_ema_init_f32(dsp_ema_f32_t *p, dsp_f32_t alpha);
dsp_err_t dsp_ema_set_alpha_f32(dsp_ema_f32_t *p, dsp_f32_t alpha);
dsp_err_t dsp_ema_process_f32(dsp_ema_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_ema_reset_f32(dsp_ema_f32_t *p);

#if DSP_USE_SHORT_ALIASES
  #define SMA_Init   dsp_sma_init_f32
  #define SMA_Process dsp_sma_process_f32
  #define EMA_Init   dsp_ema_init_f32
  #define EMA_Process dsp_ema_process_f32
#endif

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_MOVING_AVERAGE */
#endif
