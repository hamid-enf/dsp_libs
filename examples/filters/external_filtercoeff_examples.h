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
 *   - عنوان خام FIRها «21 taps» بود، اما بخش Design و آرایه‌ی خروجی هر سه
 *     فیلتر 31 ضریب دارد؛ مقدار معتبر 31 در این header استفاده شده است.
 *   - ضرایب IIR از قبل در قالب SOS کتابخانه‌ی فعلی هستند:
 *       [b0, b1, b2, a1, a2]
 *     یعنی a0 قبلاً نرمال و حذف شده است.
 */
#ifndef DSP_EXTERNAL_FILTERCOEFF_EXAMPLES_H
#define DSP_EXTERNAL_FILTERCOEFF_EXAMPLES_H

#include "dsp.h"

#if DSP_ENABLE_FIR
#define DSP_FILTERCOEFF_FIR_TAPS  31u
#define DSP_FILTERCOEFF_FIR_BLOCK 128u

/*
 * کاربردهای نمونه:
 *   FIRهای 1k/2k/5k: low-pass برای صوت یا ADC وقتی phase خطی مهم است؛
 *                    cutoff انتخابی باید با پهنای باند واقعی سیگنال مچ شود.
 *   IIR Butterworth order 2: low-pass کم‌هزینه برای مسیر Real-Time.
 *   IIR Butterworth order 6: حذف تندتر خارج از باند برای صوت/ADC؛ فقط SOS.
 */

/* Fs=48kHz, Fc=1kHz, 31 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_1k[DSP_FILTERCOEFF_FIR_TAPS] = {
     0.0025629260f,  0.0032317041f,  0.0047510757f,  0.0072755819f,
     0.0108857896f,  0.0155749786f,  0.0212430352f,  0.0276983808f,
     0.0346680406f,  0.0418152053f,  0.0487629349f,  0.0551220618f,
     0.0605209333f,  0.0646344272f,  0.0672097057f,  0.0680864387f,
     0.0672097057f,  0.0646344272f,  0.0605209333f,  0.0551220618f,
     0.0487629349f,  0.0418152053f,  0.0346680406f,  0.0276983808f,
     0.0212430352f,  0.0155749786f,  0.0108857896f,  0.0072755819f,
     0.0047510757f,  0.0032317041f,  0.0025629260f
};

/* Fs=48kHz, Fc=2kHz, 31 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_2k[DSP_FILTERCOEFF_FIR_TAPS] = {
    -0.0012867436f, -0.0010973474f, -0.0008135906f,  5.8447359e-19f,
     0.0018641202f,  0.0052885911f,  0.0106652866f,  0.0181694042f,
     0.0276880725f,  0.0387914072f,  0.0507542939f,  0.0626285540f,
     0.0733562737f,  0.0819075588f,  0.0874212192f,  0.0893258007f,
     0.0874212192f,  0.0819075588f,  0.0733562737f,  0.0626285540f,
     0.0507542939f,  0.0387914072f,  0.0276880725f,  0.0181694042f,
     0.0106652866f,  0.0052885911f,  0.0018641202f,  5.8447359e-19f,
    -0.0008135906f, -0.0010973474f, -0.0012867436f
};

/* Fs=48kHz, Fc=5kHz, 31 taps, Hamming, Unity DC gain */
static const dsp_f32_t dsp_filtercoeff_fir_lp_5k[DSP_FILTERCOEFF_FIR_TAPS] = {
    -0.0006490572f,  0.0005294277f,  0.0023244096f,  0.0044482620f,
     0.0053257484f,  0.0025515409f, -0.0053797675f, -0.0169346803f,
    -0.0264882843f, -0.0255656509f, -0.0063923861f,  0.0337014103f,
     0.0893313554f,  0.1474803207f,  0.1916479040f,  0.2081388948f,
     0.1916479040f,  0.1474803207f,  0.0893313554f,  0.0337014103f,
    -0.0063923861f, -0.0255656509f, -0.0264882843f, -0.0169346803f,
    -0.0053797675f,  0.0025515409f,  0.0053257484f,  0.0044482620f,
     0.0023244096f,  0.0005294277f, -0.0006490572f
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

/* Existing direct order-2 variant, Fs=48kHz, Butterworth Low-Pass, Fc=2kHz */
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

/* Fs=48kHz, Butterworth Low-Pass, order 6, Fc=2kHz, 3 SOS sections */
static const dsp_f32_t dsp_filtercoeff_iir_bw6_lp_2k[15] = {
     3.13420459e-06f,  6.26840918e-06f,  3.13420459e-06f,
    -1.5454813221f,     0.6000000000f,
     1.0f,              2.0f,             1.0f,
    -1.6329931619f,     0.6905989232f,
     1.0f,              2.0f,             1.0f,
    -1.8105666825f,     0.8744365594f
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
