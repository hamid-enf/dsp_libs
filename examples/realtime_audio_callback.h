/*
 * realtime_audio_callback.h — الگوی آماده برای اتصال DSP به PCM tap
 *
 * این مثال عمداً فقط عملیات bounded و بدون malloc/printf انجام می‌دهد:
 *   DC Blocker -> Notch -> Low-Pass Biquad -> Limiter
 * برای هر کانال state جدا دارد و بافر PCM را in-place تغییر می‌دهد.
 *
 * استفاده:
 *   1) dsp_example_audio_init(rate) را بعد از هر SimpleAudio_Begin صدا بزن.
 *   2) داخل pre/post PCM tap فقط dsp_example_audio_process_pcm16 را صدا بزن.
 *   3) اگر نرخ/تعداد کانال عوض شد، بیرون callback دوباره init کن.
 */
#ifndef DSP_EXAMPLE_REALTIME_AUDIO_CALLBACK_H
#define DSP_EXAMPLE_REALTIME_AUDIO_CALLBACK_H

#include "dsp.h"

#if DSP_ENABLE_DCBLOCK && DSP_ENABLE_NOTCH && DSP_ENABLE_BIQUAD && DSP_ENABLE_SIGNAL_UTILS
#define DSP_EXAMPLE_AUDIO_MAX_CHANNELS 2u

typedef struct {
    dsp_dcblock_f32_t dc[DSP_EXAMPLE_AUDIO_MAX_CHANNELS];
    dsp_notch_f32_t notch[DSP_EXAMPLE_AUDIO_MAX_CHANNELS];
    dsp_biquad_f32_t lowpass[DSP_EXAMPLE_AUDIO_MAX_CHANNELS];
    dsp_limiter_f32_t limiter[DSP_EXAMPLE_AUDIO_MAX_CHANNELS];
    uint32_t sample_rate;
    uint16_t channels;
    uint8_t initialized;
} dsp_example_audio_context_t;

static dsp_example_audio_context_t dsp_example_audio;

static inline dsp_err_t dsp_example_audio_init(uint32_t sample_rate,
                                                uint16_t channels)
{
    uint16_t ch;
    dsp_err_t e;
    if (sample_rate == 0u || channels == 0u || channels > DSP_EXAMPLE_AUDIO_MAX_CHANNELS) {
        return DSP_ERR_INVALID_PARAMETER;
    }

    /* ابتدا initialized را خاموش کن تا callback نیمه‌راه فیلتر را اجرا نکند. */
    dsp_example_audio.initialized = 0u;
    dsp_example_audio.sample_rate = sample_rate;
    dsp_example_audio.channels = channels;
    for (ch = 0u; ch < channels; ++ch) {
        e = dsp_dcblock_init_f32(&dsp_example_audio.dc[ch], 0.995f);
        if (e != DSP_OK) return e;
        e = dsp_notch_init_f32(&dsp_example_audio.notch[ch], sample_rate, 50.0f, 20.0f);
        if (e != DSP_OK) return e;
        e = dsp_biquad_init_f32(&dsp_example_audio.lowpass[ch], DSP_TDF2);
        if (e != DSP_OK) return e;
        e = dsp_biquad_set_params_f32(&dsp_example_audio.lowpass[ch],
                                      DSP_BIQUAD_LPF, sample_rate,
                                      16000.0f, 0.707f, 0.0f);
        if (e != DSP_OK) return e;
        e = dsp_limiter_init_f32(&dsp_example_audio.limiter[ch],
                                 0.90f, 0.98f, 0.20f, 0.002f);
        if (e != DSP_OK) return e;
    }
    dsp_example_audio.initialized = 1u;
    return DSP_OK;
}

/*
 * sample_count تعداد مقدارهای PCM است، نه تعداد frameها:
 *   stereo: sample_count = frames * 2
 *   mono:   sample_count = frames
 * اگر tap شما frames می‌دهد، قبل از فراخوانی آن را در channels ضرب کنید.
 */
static inline dsp_err_t dsp_example_audio_process_pcm16(int16_t *pcm,
                                                         uint32_t sample_count,
                                                         uint16_t channels)
{
    uint32_t i;
    if (pcm == NULL) return DSP_ERR_NULL_PTR;
    if (!dsp_example_audio.initialized) return DSP_ERR_NOT_INITIALIZED;
    if (channels == 0u || channels != dsp_example_audio.channels ||
        channels > DSP_EXAMPLE_AUDIO_MAX_CHANNELS) return DSP_ERR_INVALID_PARAMETER;
    if ((sample_count % channels) != 0u) return DSP_ERR_INVALID_PARAMETER;

    for (i = 0u; i < sample_count; ++i) {
        uint16_t ch = (uint16_t)(i % channels);
        dsp_f32_t x = (dsp_f32_t)pcm[i] / 32768.0f;
        dsp_f32_t y;
        if (dsp_dcblock_process_f32(&dsp_example_audio.dc[ch], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_notch_process_f32(&dsp_example_audio.notch[ch], y, &x) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_biquad_process_sample_f32(&dsp_example_audio.lowpass[ch], x, &y) != DSP_OK) return DSP_ERR_INVALID_STATE;
        if (dsp_limiter_process_f32(&dsp_example_audio.limiter[ch], y, &x) != DSP_OK) return DSP_ERR_INVALID_STATE;
        pcm[i] = (int16_t)dsp_clamp_f32(x * 32768.0f, -32768.0f, 32767.0f);
    }
    return DSP_OK;
}
#endif /* all modules */
#endif /* DSP_EXAMPLE_REALTIME_AUDIO_CALLBACK_H */
