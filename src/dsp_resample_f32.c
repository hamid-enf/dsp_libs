/**
 * @file dsp_resample_f32.c — Decimator / Interpolator (ساختار Polyphase)
 */
#include "dsp_resample.h"
#include "dsp_port.h"

#if DSP_ENABLE_RESAMPLING

/* ============ Decimator (ضریب D) ============
   y[m] = Σ_{k=0}^{N-1} h[k]·x[D·m − k]   (N = D·P)
   حلقه‌ی داخلی فقط P درایه در گام D — معادل بهره‌ی Polyphase. */
dsp_err_t dsp_decim_init_f32(dsp_decim_f32_t *p, uint16_t d, uint16_t tapsPerPhase,
                             const dsp_f32_t *coeffs, dsp_f32_t *state)
{
    if (p == NULL || coeffs == NULL || state == NULL) return DSP_ERR_NULL_PTR;
    if (d == 0 || tapsPerPhase == 0) return DSP_ERR_INVALID_PARAMETER;
    p->d = d;
    p->tapsPerPhase = tapsPerPhase;
    p->pCoeffs = coeffs;
    p->pState = state;
    p->writePos = 0;
    p->totalTaps = (uint32_t)d * tapsPerPhase;
    dsp_decim_reset_f32(p);
    return DSP_OK;
}

dsp_err_t dsp_decim_process_f32(dsp_decim_f32_t *p, const dsp_f32_t *src,
                                dsp_f32_t *dst, uint16_t nOut)
{
    uint16_t m;
    uint32_t taps;
    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;
    taps = p->totalTaps;

    /* به ازای هر خروجی، D نمونه‌ی ورودی مصرف می‌شود:
       y[m] = Σ_{k=0}^{N-1} h[k]·x[D·m − k]
       (کانولوشن مستقیم در نرخ خروجی — یعنی همان بهره‌ی Polyphase، چون
       خروجی‌های دورانداخته‌شده محاسبه نمی‌شوند) */
    for (m = 0; m < nOut; m++) {
        dsp_f32_t acc = 0.0f;
        uint32_t k;
        uint16_t in_i;
        /* قبل از خروجی m، نمونه‌های x[mD−D+1 .. mD] مصرف می‌شوند
           (یعنی آخرین D نمونه‌ای که شاخص‌شان ≤ mD است) */
        for (in_i = 0; in_i < p->d; in_i++) {
            int32_t srcIdx = (int32_t)m * (int32_t)p->d - (int32_t)p->d + 1 + (int32_t)in_i;
            p->pState[p->writePos] = (srcIdx >= 0) ? src[srcIdx] : 0.0f;
            p->writePos++;
            if (p->writePos >= taps) p->writePos = 0;
        }
        for (k = 0; k < taps; k++) {
            int32_t idx = (int32_t)p->writePos - 1 - (int32_t)k;
            if (idx < 0) idx += (int32_t)taps;
            acc += p->pCoeffs[k] * p->pState[idx];
        }
        dst[m] = acc;
    }
    return DSP_OK;
}

dsp_err_t dsp_decim_reset_f32(dsp_decim_f32_t *p)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->writePos = 0;
    if (p->pState != NULL) {
        uint32_t i;
        for (i = 0; i < p->totalTaps; i++) p->pState[i] = 0.0f;
    }
    return DSP_OK;
}

/* ============ Interpolator (ضریب L) ============
   y[m] = Σ_j h[p + j·L]·x[n0 − j]   ،  p = m mod L ، n0 = m/L
   فقط P ضرب‌جمع به‌ازای هر خروجی (صفر-گذاری حذف شده است). */
dsp_err_t dsp_interp_init_f32(dsp_interp_f32_t *p, uint16_t l, uint16_t tapsPerPhase,
                              const dsp_f32_t *coeffs, dsp_f32_t *state)
{
    if (p == NULL || coeffs == NULL || state == NULL) return DSP_ERR_NULL_PTR;
    if (l == 0 || tapsPerPhase == 0) return DSP_ERR_INVALID_PARAMETER;
    p->l = l;
    p->tapsPerPhase = tapsPerPhase;
    p->pCoeffs = coeffs;
    p->pState = state;
    p->idx = 0;
    dsp_interp_reset_f32(p);
    return DSP_OK;
}

dsp_err_t dsp_interp_process_f32(dsp_interp_f32_t *p, const dsp_f32_t *src,
                                 dsp_f32_t *dst, uint16_t nIn)
{
    uint16_t i, ph, P = p->tapsPerPhase;

    if (p == NULL || src == NULL || dst == NULL) return DSP_ERR_NULL_PTR;

    /* به ازای هر ورودی x[i]، L خروجی تولید می‌شود (پلی‌فاز واقعی):
       y[i·L + ph] = Σ_{k=0}^{P-1} h[ph + k·L] · x[i − k]
       (ضرایب در نظم طبیعی h[0..L·P-1] هستند) */
    for (i = 0; i < nIn; i++) {
        p->pState[p->idx] = src[i];
        p->idx++;
        if (p->idx >= P) p->idx = 0;
        for (ph = 0; ph < p->l; ph++) {
            dsp_f32_t acc = 0.0f;
            uint16_t k;
            for (k = 0; k < P; k++) {
                int32_t ri = (int32_t)p->idx - 1 - (int32_t)k;
                dsp_f32_t h;
                if (ri < 0) ri += P;
                h = p->pCoeffs[(uint32_t)ph + (uint32_t)k * p->l];
                acc += h * p->pState[ri];
            }
            dst[(uint32_t)i * p->l + ph] = acc;
        }
    }
    return DSP_OK;
}

dsp_err_t dsp_interp_reset_f32(dsp_interp_f32_t *p)
{
    uint16_t i;
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->idx = 0;
    if (p->pState != NULL) {
        for (i = 0; i < p->tapsPerPhase; i++) p->pState[i] = 0.0f;
    }
    return DSP_OK;
}

/* طراح پروتوتایپ ساده: sinc پنجره‌دار (همان تقریب Low-Pass) */
void dsp_resample_design_lp_f32(dsp_f32_t *h, uint16_t taps, dsp_f32_t cutoff)
{
    uint16_t i;
    dsp_f32_t sum = 0.0f;
    dsp_f32_t mid = (dsp_f32_t)(taps - 1) * 0.5f;
    for (i = 0; i < taps; i++) {
        dsp_f32_t x = (dsp_f32_t)i - mid;
        dsp_f32_t v;
        if (x == 0.0f) {
            v = 2.0f * cutoff;
        } else {
            v = dsp_sin_f32(2.0f * DSP_PI_F * cutoff * x) / (DSP_PI_F * x);
        }
        /* پنجره‌ی هنینگ */
        v *= 0.5f - 0.5f * dsp_cos_f32(DSP_TWO_PI_F * (dsp_f32_t)i / (dsp_f32_t)(taps - 1));
        h[i] = v;
        sum += v;
    }
    for (i = 0; i < taps; i++) h[i] /= sum;
}

#endif /* DSP_ENABLE_RESAMPLING */
