# مثال‌های ملموس فیلترها

این پوشه فقط مثال است و به `inc/` و `src/` کتابخانه دست نمی‌زند. هر header
یک context و state استاتیک دارد تا بتوان آن را سریعاً در یک `main.c` یا فایل
کاربردی کپی کرد. نام فایل‌ها و کاربردشان:

| فایل | کاربرد اصلی |
|---|---|
| `filters/01_fir_example.h` | Low-pass FIR، مناسب smoothing و حذف نویز |
| `filters/02_iir_example.h` | IIR مستقیم مرتبه ۲ با TDF2 برای صدای کم‌هزینه |
| `filters/03_biquad_example.h` | اکولایزر سه‌باند استریو با PCM16 interleaved |
| `filters/04_butterworth_example.h` | طراحی Butterworth و اجرای SOS |
| `filters/05_chebyshev_example.h` | مقایسه‌ی Chebyshev نوع I و II |
| `filters/06_elliptic_example.h` | شیب قطع بسیار تند با ripple در دو باند |
| `filters/07_bessel_example.h` | گذرای نرم و Group Delay بهتر |
| `filters/08_moving_average_example.h` | SMA، WMA و EMA برای smoothing |
| `filters/09_median_example.h` | حذف spike از ADC/سنسور |
| `filters/10_savgol_example.h` | smoothing با حفظ شکل و مشتق |
| `filters/11_dc_blocker_example.h` | حذف offset میکروفون/ADC |
| `filters/12_notch_example.h` | حذف hum برق ۵۰/۶۰ هرتز |
| `filters/13_adaptive_example.h` | حذف نویز با LMS/NLMS و دو میکروفون |
| `filters/14_kalman_example.h` | تخمین نرم از سنسور noisy |
| `filters/15_resampling_example.h` | decimation و interpolation |
| `filters/16_signal_utils_example.h` | gain، RMS، envelope و limiter |
| `filters/external_filtercoeff_examples.h` | ضرایب تمیزشده‌ی خروجی FilterCoeff |
| `realtime_audio_callback.h` | زنجیره‌ی کامل PCM16 برای callback صوتی |

## الگوی مشترک: Initialize یک‌بار، Process در callback

```c
#include "dsp.h"
#include "01_fir_example.h"

void App_AudioInit(void)
{
    if (dsp_example_fir_init() != DSP_OK) {
        /* خطا را بیرون از callback گزارش/مدیریت کنید. */
    }
}

/* callback: بدون malloc، printf، فایل، HAL_Delay یا طراحی ضرایب */
void App_AudioCallback(const float *input, float *output, uint16_t frames)
{
    (void)dsp_example_fir_process_block(input, output, frames);
}
```

فیلترهای stateful هستند. state را بین callbackها نگه دارید و برای هر کانال
یک instance جدا داشته باشید؛ یک فیلتر مشترک برای L و R باعث مخلوط‌شدن تاریخچه‌ی
دو کانال می‌شود. اگر callback شما `int16_t` است، قبل از فیلتر به بازه‌ی
تقریباً `[-1, +1]` تبدیل کنید و بعد از فیلتر با clamp به PCM برگردانید.

## اتصال به PCM tap پروژه‌ی صدا

فایل `realtime_audio_callback.h` دقیقاً همین الگو را برای بافر `int16_t`
پیاده می‌کند:

```c
#include "realtime_audio_callback.h"

/* بعد از هر SimpleAudio_Begin، چون Begin ممکن است tap/state را پاک کند. */
void Music_InstallDsp(uint32_t rate, uint16_t channels)
{
    (void)dsp_example_audio_init(rate, channels);
}

static void my_audio_pre_tap(void *ctx, int16_t *pcm, size_t samples,
                             uint32_t rate, uint16_t channels)
{
    (void)ctx;
    (void)rate; /* اگر rate عوض شد، در مسیر اصلی re-init کنید، نه اینجا. */
    (void)dsp_example_audio_process_pcm16(pcm, (uint32_t)samples, channels);
}
```

در این قرارداد `samples` باید تعداد مقدارهای `int16_t` باشد؛ برای استریو یعنی
`frames * 2`. اگر کتابخانه‌ی صوتی شما `frames` را تحویل می‌دهد، ابتدا آن را
در `channels` ضرب کنید. همچنین اگر tap بعد از volume نصب شود، پردازش روی چیزی
انجام می‌شود که واقعاً به اسپیکر می‌رود؛ اگر قبل از volume نصب شود، ولوم بعدی
نتیجه را تغییر می‌دهد.

کارهای سنگین و غیر Real-Time را از callback بیرون نگه دارید:

- طراحی Butterworth/Chebyshev/Elliptic/Bessel و تغییر ضرایب: فقط در init یا task
  کنترل، نه وسط DMA callback.
- `printf`، فایل، allocation، mutex و `HAL_Delay`: هرگز در callback.
- VU/FFT/ضبط: در callback فقط peak یا داده‌ی کوچک را در یک ring buffer/flag
  بنویسید؛ گزارش و FFT را در task اصلی انجام دهید.
- اگر بافر DMA در AXI-SRAM است، cache maintenance را طبق پروژه‌ی STM32 خودتان
  انجام دهید؛ state فیلتر که فقط CPU استفاده می‌کند می‌تواند در DTCM باشد.

## طراحی فیلترهای مرتبه بالا

طراحان خانواده‌ی `04` تا `07` در زمان init یک خروجی SOS می‌سازند، آن را به
`float32` تبدیل می‌کنند و با `dsp_iir_init_f32(..., DSP_TDF2, ..., sos=1)`
راه می‌اندازند. برای صوت محصولی، بعد از انتخاب `Fs` و cutoff ضرایب را یک بار
Offline تولید و در Flash ذخیره کنید. در callback فقط
`dsp_iir_process_sample_f32` یا `dsp_iir_process_block_f32` باقی می‌ماند.

## اجرای همه‌ی مثال‌ها در `Core/Src/main.c`

فایل `Core/Src/main.c` ساختار CubeMX را حفظ می‌کند: `MPU_Config`، `HAL_Init`،
`SystemClock_Config`، راه‌اندازی GPIO/UART و حلقه‌ی اصلی همچنان سر جای خود
هستند. فقط بخش‌های `USER CODE` گسترش داده شده‌اند تا همه‌ی headerهای بالا را
یک‌بار initialize و سپس تست کنند.

بعد از reset برد، خروجی UART شامل دو بخش است:

```text
--- FILTER INITIALIZATION TIME ---
[TIME] Butterworth init : ... cycles/call | ... us/call | DSP_OK

--- FILTER PROCESSING TIME ---
[TIME] FIR process block : ... cycles/call | ... us/call | DSP_OK
[TIME] LMS process       : ... cycles/call | ... us/call | DSP_OK
```

اندازه‌گیری با شمارنده‌ی `DWT->CYCCNT` انجام می‌شود و هر تابع پردازشی چند بار
اجرا می‌شود تا میانگین پایدارتر به دست آید. برای wrapperهایی که یک بلاک را
پردازش می‌کنند، زمان به ازای همان API گزارش می‌شود؛ برای wrapperهای sample،
زمان به ازای هر sample گزارش می‌شود. تبدیل cycle به microsecond بر اساس
`SystemCoreClock` بعد از `SystemClock_Config` انجام می‌شود.

این بخش benchmark اولیه است و نباید داخل callback واقعی صوت قرار بگیرد؛ در
callback فقط `process` فیلتر را اجرا کنید.

## ارزیابی خروجی FilterCoeff

خروجی خامی که از `FilterCoeff v1.0.0` گرفته شده بود، از نظر طراحی معتبر است،
اما بخش FIR آن برای قرار دادن مستقیم در C تمیز نبود: comma بین ضرایب چاپ نشده
بود و نوع `float64_t` با مسیر سریع f32 کتابخانه‌ی STM32H7 یکی نبود. فایل
`filters/external_filtercoeff_examples.h` نسخه‌ی تمیز و قابل استفاده را دارد.

نکته‌ی مهم درباره‌ی گزارش جدید: در عنوان هر FIR عبارت `21 taps` آمده، اما در
بخش `Design` مقدار `Taps = 31` است و آرایه‌ی چاپ‌شده نیز دقیقاً ۳۱ ضریب دارد.
بنابراین در header مقدار معتبر ۳۱ استفاده شده است؛ تأخیر گروهی فیلتر Type 1
برابر `15` نمونه، یعنی `15 / 48000 = 0.3125 ms` است. برای مسیر Direct یا
Symmetric با block size برابر ۱۲۸، حداقل state برابر
`31 + 128 - 1 = 158` مقدار `dsp_f32_t` است؛ این state باید بیرون callback
رزرو و initialize شود.

جمع‌بندی فنی:

- هر سه آرایه‌ی FIR symmetric و با Unity DC gain هستند؛ برای ۳۱ tap می‌توان
  از `DSP_FORM_SYMMETRIC` استفاده کرد.
- معیارهای گزارش‌شده‌ی FIR عبارت‌اند از: `1kHz`: ripple گزارش‌شده‌ی
  `2.358326 dB` و stopband برابر `50.2899 dB`؛ `2kHz`: به‌ترتیب
  `5.389732 dB` و `55.0192 dB`؛ `5kHz`: به‌ترتیب `6.037791 dB` و
  `57.2971 dB`. مقدار passband را بدون دانستن تعریف دقیق FilterCoeff از نظر
  پهنای passband، ripple ایده‌آل فرض نکن.
- ۳۱ tap نسبت به ۲۱ tap هزینه و تأخیر بیشتری دارد و معمولاً آزادی بیشتری برای
  transition و stopband می‌دهد، اما همین خروجی را فقط بر اساس عدد ripple بهتر
  مطلق ندان؛ تعریف passband در گزارش مشخص نیست و برای cutoff پایین هنوز محدودیت
  وجود دارد. اگر passband بسیار صاف، شیب تند یا stopband مشخص لازم است، tapها
  را با مشخصات واقعی سیستم دوباره طراحی کن.
- IIR Butterworth مرتبه ۲ با `1kHz` دقیقاً با آرایه‌ی مستقیم مثال IIR مچ
  می‌شود. هر سه IIR مرتبه ۶ (`1kHz`، `2kHz` و `5kHz`) سه SOS دارند و باید
  حتماً با `DSP_TDF2` و `sos=1` اجرا شوند، نه به‌صورت Direct polynomial.
- مقدار `3.010300 dB` برای Butterworth ripple واقعی نیست؛ افت ذاتی Butterworth
  در cutoff حدود ۳ dB است. مقدار `-6000 dB` در Nyquist نیز کف گزارش/نمایش
  عددی است، نه اندازه‌گیری فیزیکی با دقت واقعی تا شش هزار دسی‌بل.

برای صدای Real-Time، نقطه‌ی شروع پیشنهادی:

```c
/* کم‌هزینه و ساده */
static dsp_f32_t iir_state_2[2];
dsp_iir_f32_t iir_2;
dsp_filtercoeff_iir_init_sos_f32(
    &iir_2, dsp_filtercoeff_iir_bw2_lp_1k,
    2u, iir_state_2, 2u);

/* شیب قطع بیشتر، با سه SOS */
static dsp_f32_t iir_state_6[6];
dsp_iir_f32_t iir_6;
dsp_filtercoeff_iir_init_sos_f32(
    &iir_6, dsp_filtercoeff_iir_bw6_lp_1k,
    6u, iir_state_6, 6u);
```

در هر دو حالت، داخل callback فقط `dsp_iir_process_sample_f32` یا
`dsp_iir_process_block_f32` را اجرا کن.

## تست روی Host

از ریشه‌ی کتابخانه:

```bash
cd dsp_libs
make test
```

این تست‌های موجود کتابخانه را اجرا می‌کند؛ headerهای مثال به HAL وابسته نیستند
و برای اتصال به target طراحی شده‌اند. برای تست سریع روی Host نیز می‌توان یک
فایل C کوچک با `-Iinc -Iexamples/filters` ساخت و یکی از headerها را include کرد.
