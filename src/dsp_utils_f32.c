/**
 * @file dsp_utils_f32.c — بلوک‌های پردازش سیگنال عمومی
 */
#include "dsp_utils.h"
#include "dsp_port.h"

#if DSP_ENABLE_SIGNAL_UTILS

void dsp_gain_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n, dsp_f32_t gain,
                  dsp_bool_t saturate)
{
    uint16_t i;
    if (src == NULL || dst == NULL) return;
    for (i = 0; i < n; i++) {
        dsp_f32_t v = src[i] * gain;
        if (saturate) v = dsp_clamp_f32(v, -1.0f, 1.0f);
        dst[i] = v;
    }
}

dsp_f32_t dsp_normalize_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    dsp_f32_t peak = 0.0f;
    if (src == NULL || dst == NULL) return 0.0f;
    for (i = 0; i < n; i++) {
        dsp_f32_t a = dsp_abs_f32(src[i]);
        if (a > peak) peak = a;
    }
    if (peak > 0.0f) {
        dsp_f32_t inv = 1.0f / peak;
        for (i = 0; i < n; i++) dst[i] = src[i] * inv;
    } else {
        for (i = 0; i < n; i++) dst[i] = src[i];
    }
    return peak;
}

void dsp_saturate_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n, dsp_f32_t limit)
{
    uint16_t i;
    if (src == NULL || dst == NULL) return;
    if (limit <= 0.0f) return;
    for (i = 0; i < n; i++) {
        dsp_f32_t v = src[i];
        if (v > limit) v = limit;
        else if (v < -limit) v = -limit;
        dst[i] = v;
    }
}

void dsp_clip_f32(const dsp_f32_t *src, dsp_f32_t *dst, uint16_t n,
                  dsp_f32_t lo, dsp_f32_t hi)
{
    uint16_t i;
    if (src == NULL || dst == NULL) return;
    for (i = 0; i < n; i++) dst[i] = dsp_clamp_f32(src[i], lo, hi);
}

dsp_f32_t dsp_rms_f32(const dsp_f32_t *src, uint16_t n)
{
    uint16_t i;
    double acc = 0.0;
    if (src == NULL || n == 0) return 0.0f;
    for (i = 0; i < n; i++) acc += (double)src[i] * (double)src[i];
    return (dsp_f32_t)sqrt(acc / (double)n);
}

/* ---- Peak Detector ---- */
void dsp_peak_init_f32(dsp_peak_f32_t *p, dsp_f32_t attack, dsp_f32_t release)
{
    if (p == NULL) return;
    p->attack = attack;
    p->release = release;
    p->peak = 0.0f;
}

dsp_err_t dsp_peak_process_f32(dsp_peak_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    dsp_f32_t a = dsp_abs_f32(x);
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    if (a > p->peak) {
        p->peak = p->attack * a + (1.0f - p->attack) * p->peak;
    } else {
        p->peak = p->release * p->peak;
    }
    *y = p->peak;
    return DSP_OK;
}

/* ---- Envelope Follower ---- */
void dsp_env_init_f32(dsp_env_f32_t *p, dsp_f32_t attack, dsp_f32_t release)
{
    if (p == NULL) return;
    p->attack = attack;
    p->release = release;
    p->env = 0.0f;
}

dsp_err_t dsp_env_process_f32(dsp_env_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    dsp_f32_t a = dsp_abs_f32(x);
    dsp_f32_t c;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    c = (a > p->env) ? p->attack : p->release;
    p->env = c * a + (1.0f - c) * p->env;
    *y = p->env;
    return DSP_OK;
}

/* ---- Limiter ---- */
dsp_err_t dsp_limiter_init_f32(dsp_limiter_f32_t *p, dsp_f32_t threshold,
                               dsp_f32_t ceiling, dsp_f32_t attack, dsp_f32_t release)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (threshold <= 0.0f || ceiling < threshold || ceiling <= 0.0f) {
        return DSP_ERR_INVALID_PARAMETER;
    }
    p->threshold = threshold;
    p->ceiling = ceiling;
    p->attack = attack;
    p->release = release;
    p->gain = 1.0f;
    return DSP_OK;
}

dsp_err_t dsp_limiter_process_f32(dsp_limiter_f32_t *p, dsp_f32_t x, dsp_f32_t *y)
{
    dsp_f32_t a, target;
    if (p == NULL || y == NULL) return DSP_ERR_NULL_PTR;
    a = dsp_abs_f32(x);
    /* گین هدف: بالای آستانه، برای رسیدن به سقف */
    if (a > p->threshold) {
        target = (a * p->gain > p->ceiling) ? (p->ceiling / a) : p->gain;
        /* حمله‌ی نرم */
        p->gain += p->attack * (target - p->gain);
    } else {
        /* رهاسازی به سمت 1 */
        p->gain += p->release * (1.0f - p->gain);
    }
    *y = x * p->gain;
    return DSP_OK;
}

dsp_err_t dsp_limiter_process_block_f32(dsp_limiter_f32_t *p, const dsp_f32_t *src,
                                        dsp_f32_t *dst, uint16_t n)
{
    uint16_t i;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    for (i = 0; i < n; i++) {
        dsp_err_t e = dsp_limiter_process_f32(p, src[i], &dst[i]);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

dsp_err_t dsp_limiter_reset_f32(dsp_limiter_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->gain = 1.0f;
    return DSP_OK;
}

#endif /* DSP_ENABLE_SIGNAL_UTILS */
