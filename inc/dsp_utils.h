/**
 * @file dsp_utils.h — بلوک‌های پردازش سیگنال عمومی (Optional)
 * Gain / Scale / Normalize / Saturate / Clip / RMS / Peak / Envelope / Limiter
 */
#ifndef DSP_UTILS_H
#define DSP_UTILS_H
#include "dsp_types.h"
#if DSP_ENABLE_SIGNAL_UTILS
#ifdef __cplusplus
extern "C" {
#endif

/* Gain با اشباع اختیاری */
void dsp_gain_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n, dsp_f32_t gain,
                  dsp_bool_t saturate);

/* مقیاس‌دهی بر اساس بیشینه‌ی قدرمطلق (Normalize به ±1) */
dsp_f32_t dsp_normalize_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n);

/* اشباع/کلیپ */
void dsp_saturate_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n, dsp_f32_t limit);
void dsp_clip_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n,
                  dsp_f32_t lo, dsp_f32_t hi);

/* RMS */
dsp_f32_t dsp_rms_f32(const dsp_f32_t *src, uint16_t n);

/* Peak Detector با Attack/Release */
typedef struct {
    dsp_f32_t attack;   /* ضریب حمله (0..1، نزدیک 1 = سریع) */
    dsp_f32_t release;  /* ضریب رهاسازی */
    dsp_f32_t peak;
} dsp_peak_f32_t;
void dsp_peak_init_f32(dsp_peak_f32_t *p, dsp_f32_t attack, dsp_f32_t release);
dsp_err_t dsp_peak_process_f32(dsp_peak_f32_t *p, dsp_f32_t x, dsp_f32_t *y);

/* Envelope Follower (یک‌قطبی‌های حمله/رهاسازی) */
typedef struct {
    dsp_f32_t attack;
    dsp_f32_t release;
    dsp_f32_t env;
} dsp_env_f32_t;
void dsp_env_init_f32(dsp_env_f32_t *p, dsp_f32_t attack, dsp_f32_t release);
dsp_err_t dsp_env_process_f32(dsp_env_f32_t *p, dsp_f32_t x, dsp_f32_t *y);

/* Limiter — کاهش گین بر اساس Peak با ضرایب حمله/رهاسازی */
typedef struct {
    dsp_f32_t threshold;  /* آستانه (0..1) */
    dsp_f32_t ceiling;    /* سقف خروجی (≥ threshold) */
    dsp_f32_t attack;
    dsp_f32_t release;
    dsp_f32_t gain;       /* گین جاری */
} dsp_limiter_f32_t;
dsp_err_t dsp_limiter_init_f32(dsp_limiter_f32_t *p, dsp_f32_t threshold,
                               dsp_f32_t ceiling, dsp_f32_t attack, dsp_f32_t release);
dsp_err_t dsp_limiter_process_f32(dsp_limiter_f32_t *p, dsp_f32_t x, dsp_f32_t *y);
dsp_err_t dsp_limiter_process_block_f32(dsp_limiter_f32_t *p, const dsp_f32_t *src,
                                        dsp_f32_t *dst, uint16_t n);
dsp_err_t dsp_limiter_reset_f32(dsp_limiter_f32_t *p);

#ifdef __cplusplus
}
#endif
#endif /* DSP_ENABLE_SIGNAL_UTILS */
#endif
