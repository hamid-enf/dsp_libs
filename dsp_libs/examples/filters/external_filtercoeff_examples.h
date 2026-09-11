/*
 * external_filtercoeff_examples.h
 *
 * ضرایب تمیزشده از گزارش FilterCoeff v1.0.0.
 *
 * تفاوت با خروجی خام گزارش:
 *   - timestamp، Markdown و متن گزارش حذف شده‌اند.
 *   - commaهای فراموش‌شده به آرایه‌های FIR اضافه شده‌اند.
 *   - float64_t به dsp_f32_t تبدیل شده است تا مستقیم با مسیر سریع f32
 *     کتابخانه‌ی STM32H7 استفاده شود.
 *   - ضرایب IIR از قبل در قالب SOS کتابخانه‌ی فعلی هستند:
 *       [b0, b1, b2, a1, a2]
 *     یعنی a0 قبلاً نرمال و حذف شده است.
 */
#ifndef DSP_EXTERNAL_FILTERCOEFF_EXAMPLES_H
#define DSP_EXTERNAL_FILTERCOEFF_EXAMPLES_H

#include "dsp.h"

#if DSP_ENABLE_FIR
#define DSP_FILTERCOEFF_FIR_TAPS  21u
#define DSP_FILTERCOEFF_FIR_BLOCK 128u

/* Fs=48kHz, Fc=1kHz, 21 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_1k[DSP_FILTERCOEFF_FIR_TAPS] = {
     0.0056875031f,  0.0077454009f,  0.0133738213f,  0.0224908898f,
     0.0345098740f,  0.0483903497f,  0.0627590534f,  0.0760844051f,
     0.0868799047f,  0.0939065945f,  0.0963444071f,  0.0939065945f,
     0.0868799047f,  0.0760844051f,  0.0627590534f,  0.0483903497f,
     0.0345098740f,  0.0224908898f,  0.0133738213f,  0.0077454009f,
     0.0056875031f
};

/* Fs=48kHz, Fc=2kHz, 21 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_2k[DSP_FILTERCOEFF_FIR_TAPS] = {
     0.0016825320f,  0.0033878878f,  0.0076431253f,  0.0156494553f,
     0.0278916262f,  0.0438804300f,  0.0621230079f,  0.0803445559f,
     0.0959198681f,  0.1064167807f,  0.1101214618f,  0.1064167807f,
     0.0959198681f,  0.0803445559f,  0.0621230079f,  0.0438804300f,
     0.0278916262f,  0.0156494553f,  0.0076431253f,  0.0033878878f,
     0.0016825320f
};

/* Fs=48kHz, Fc=5kHz, 21 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_5k[DSP_FILTERCOEFF_FIR_TAPS] = {
     0.0006560513f, -0.0013811217f, -0.0057573051f, -0.0120996354f,
    -0.0148561785f, -0.0044665586f,  0.0270171863f,  0.0790742476f,
     0.1395823857f,  0.1885425097f,  0.2073768371f,  0.1885425097f,
     0.1395823857f,  0.0790742476f,  0.0270171863f, -0.0044665586f,
    -0.0148561785f, -0.0120996354f, -0.0057573051f, -0.0013811217f,
     0.0006560513f
};

/*
 * حالت DSP_FORM_SYMMETRIC فقط وقتی استفاده شود که ضرایب واقعاً symmetric
 * باشند؛ هر سه آرایه‌ی بالا symmetric هستند.
 */
static inline dsp_err_t dsp_filtercoeff_fir_init_f32(
    dsp_fir_f32_t *filter,
    const dsp_f32_t *coeffs,
    dsp_f32_t *state,
    uint32_t state_len,
    uint16_t block_size)
{
    return dsp_fir_init_f32(
        filter,
        coeffs,
        DSP_FILTERCOEFF_FIR_TAPS,
        block_size,
        DSP_FORM_SYMMETRIC,
        state,
        state_len,
        0u);
}
#endif /* DSP_ENABLE_FIR */

#if DSP_ENABLE_IIR
/* Fs=48kHz, Butterworth Low-Pass, order 2, Fc=1kHz */
static const dsp_f32_t dsp_filtercoeff_iir_bw2_lp_1k[5] = {
     0.0039161267f,  0.0078322533f,  0.0039161267f,
    -1.8153410827f,  0.8310055893f
};

/* Fs=48kHz, Butterworth Low-Pass, order 2, Fc=2kHz */
static const dsp_f32_t dsp_filtercoeff_iir_bw2_lp_2k[5] = {
     0.0144014403f,  0.0288028807f,  0.0144014403f,
    -1.6329931619f,  0.6905989232f
};

/* Fs=48kHz, Butterworth Low-Pass, order 6, Fc=1kHz, 3 SOS sections */
static const dsp_f32_t dsp_filtercoeff_iir_bw6_lp_1k[15] = {
     6.15535185e-08f,  1.23107037e-07f,  6.15535185e-08f,
    -1.7608803572f,     0.7760749244f,
     1.0f,              2.0f,             1.0f,
    -1.8153410827f,     0.8310055893f,
     1.0f,              2.0f,             1.0f,
    -1.9180914819f,     0.9346426177f
};

/* Fs=48kHz, Butterworth Low-Pass, order 6, Fc=5kHz, 3 SOS sections */
static const dsp_f32_t dsp_filtercoeff_iir_bw6_lp_5k[15] = {
     0.0004194894f,      0.0008389788f,     0.0004194894f,
    -0.9991739984f,      0.2594312618f,
     1.0f,               2.0f,              1.0f,
    -1.1092287926f,      0.3981522939f,
     1.0f,               2.0f,              1.0f,
    -1.3707349773f,      0.7277736259f
};

/*
 * برای SOS:
 *   order = 2 * sections
 *   state_len = 2 * sections برای DSP_TDF2
 */
static inline dsp_err_t dsp_filtercoeff_iir_init_sos_f32(
    dsp_iir_f32_t *filter,
    const dsp_f32_t *sos,
    uint16_t order,
    dsp_f32_t *state,
    uint32_t state_len)
{
    if (filter == NULL || sos == NULL || state == NULL) {
        return DSP_ERR_NULL_PTR;
    }
    if (order == 0u || (order & 1u) != 0u) {
        return DSP_ERR_INVALID_PARAMETER;
    }

    return dsp_iir_init_f32(
        filter,
        DSP_TDF2,
        order,
        sos,
        state,
        state_len,
        1u);
}
#endif /* DSP_ENABLE_IIR */

#endif /* DSP_EXTERNAL_FILTERCOEFF_EXAMPLES_H */
