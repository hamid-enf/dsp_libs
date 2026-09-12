/**
 * @file dsp_resample.h — تغییر نرخ نمونه‌برداری: Decimation / Interpolation
 *
 * ساختار Polyphase: ضرایب پروتوتایپ h (طول taps = L·P یا D·P) در نظم طبیعی
 * ذخیره می‌شوند؛ حلقه‌ی داخلی با گام L یا D آن‌ها را به‌صورت فاز-محور
 * (polyphase) مصرف می‌کند.
 *
 * Decimator (ضریب D): برای هر خروجی فقط taps ضرب-جمع (نه taps·D) — یعنی
 * همان بهره‌ی Polyphase معادل:  y[m] = Σ_k h[k]·x[D·m − k]
 */
#ifndef DSP_RESAMPLE_H
#define DSP_RESAMPLE_H
#include "dsp_types.h"
#if DSP_ENABLE_RESAMPLING
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t     d;            /* ضریب کاهش نرخ (Decim) */
    uint16_t     tapsPerPhase; /* P */
    const dsp_f32_t *pCoeffs;  /* D·P ضریب — نظم طبیعی h[0..taps-1] */
    dsp_f32_t    *pState;      /* طول D·P (حلقه) */
    uint16_t     writePos;
    uint32_t     totalTaps;
} dsp_decim_f32_t;

dsp_err_t dsp_decim_init_f32(dsp_decim_f32_t *p, uint16_t d, uint16_t tapsPerPhase,
                             const dsp_f32_t *coeffs, dsp_f32_t *state);
/* nOut خروجی تولید می‌کند و D·nOut ورودی مصرف می‌کند */
dsp_err_t dsp_decim_process_f32(dsp_decim_f32_t *p, const dsp_f32_t *src,
                                dsp_f32_t *dst, uint16_t nOut);
dsp_err_t dsp_decim_reset_f32(dsp_decim_f32_t *p);

typedef struct {
    uint16_t     l;            /* ضریب افزایش نرخ (Interp) */
    uint16_t     tapsPerPhase; /* P */
    const dsp_f32_t *pCoeffs;  /* L·P ضریب — نظم طبیعی h[0..taps-1] */
    dsp_f32_t    *pState;      /* طول P (حلقه‌ی ورودی) */
    uint16_t     idx;
} dsp_interp_f32_t;

dsp_err_t dsp_interp_init_f32(dsp_interp_f32_t *p, uint16_t l, uint16_t tapsPerPhase,
                              const dsp_f32_t *coeffs, dsp_f32_t *state);
/* nIn ورودی می‌گیرد و L·nIn خروجی تولید می‌کند */
dsp_err_t dsp_interp_process_f32(dsp_interp_f32_t *p, const dsp_f32_t *src,
                                 dsp_f32_t *dst, uint16_t nIn);
dsp_err_t dsp_interp_reset_f32(dsp_interp_f32_t *p);

/* طراح پروتوتایپ ساده: پنجره‌ی sinc (Low-Pass) با ضریب کیفیت و cutOff نرمال */
void dsp_resample_design_lp_f32(dsp_f32_t *h, uint16_t taps, dsp_f32_t cutoff);

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_RESAMPLING */
#endif
