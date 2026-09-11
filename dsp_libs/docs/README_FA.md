# کتابخانه DSP فیلتر برای STM32H7 (Cortex-M7) — نسخه 1.0.0

کتابخانه‌ی جامع، ماژولار و **Production-Ready** پردازش سیگنال دیجیتال برای
**STM32H7**، با معماری **Compile-Time Feature Selection**، چهار دقت محاسباتی،
دو Backend (داخلی/CMSIS-DSP)، **صفر تخصیص داینامیک** در حالت پیش‌فرض و
مستندات کامل فارسی.

> **شعار طراحی:** در پروژه‌ی کوچک سبک باشد، در پروژه‌ی حرفه‌ای قدرتمند باشد
> و در STM32H7 تا حد ممکن سریع، دقیق و قابل‌کنترل باشد.

---

## ۱. نمای کلی قابلیت‌ها

| دسته | ماژول | توضیح |
|------|-------|-------|
| هسته | FIR | Direct / Transposed / Symmetric + Block/Sample |
| هسته | IIR | DF1 / DF2 / TDF2 + SOS Cascade |
| هسته | Biquad | ۸ نوع فیلتر RBJ + Cascade، ضرایب قابل مشاهده/تغییر |
| طراحی | Butterworth | LP/HP/BP/BS + تخمین مرتبه |
| طراحی | Chebyshev I/II | LP/HP/BP/BS |
| طراحی | Elliptic (Cauer) | LP/HP/BP/BS — موتور کامل توابع بیضوی ژاکوبی |
| طراحی | Bessel | LP/HP/BP/BS — Group-Delay مسطح |
| کاربردی | Moving Average | SMA (O(1)) / WMA / EMA |
| کاربردی | Median | پنجره‌های ۳ تا ۳۱، نسخه f32 و Q15 |
| کاربردی | Savitzky-Golay | Smoothing + مشتق |
| کاربردی | DC Blocker | حذف مؤلفه DC |
| کاربردی | Notch | حذف ۵۰/۶۰Hz با تنظیم Runtime |
| تطبیقی | LMS / NLMS / RLS | حذف نویز/اکو، شناسایی سیستم |
| تخمین | Kalman 1D | قابل توسعه به چندبعدی |
| نرخ نمونه | Decim / Interp / SRC | ساختار Polyphase |
| عمومی | Utils | Gain/RMS/Peak/Envelope/Limiter/Saturate |

## ۲. دقت‌ها و Backend

- **دقت‌ها:** `float32` (مسیر اصلی Performance با FPU)، `float64` (اختیاری،
  فقط برای دقت بالا — در M7 نرم‌افزاری است)، `Q15` و `Q31` (Fixed-Point با
  اشباع، گردکردن و محافظت سرریز).
- **Backend:** انتخاب **در زمان کامپایل** بین `Custom (Cortex-M7 Optimized)`
  و `CMSIS-DSP`. دلیل این انتخاب (به‌جای vtable زمان اجرا) در مستندات معماری
  آمده است: صفر هزینه‌ی indirect call در مسیر Real-Time.

```c
#define DSP_USE_FLOAT32    1
#define DSP_USE_Q15        1
#define DSP_USE_Q31        1
#define DSP_USE_CMSIS_DSP  0
```

## ۳. شروع سریع (۱۰ دقیقه‌ای)

```c
#include "dsp.h"

/* ۱) تعریف ضرایب (می‌توانید با dsp_design_* تولید کنید) */
static const dsp_f32_t h[16] = { ... };            /* ضرایب FIR */
static dsp_f32_t state[16 + 64 - 1];               /* بافر حالت */

/* ۲) تعریف نمونه و Init — API ساده */
dsp_fir_t fir;
dsp_fir_init(&fir, h, 16, 64, DSP_FORM_DIRECT, state, sizeof(state)/4);

/* ۳) پردازش — نمونه‌ای یا بلاکی */
dsp_f32_t y;
dsp_fir_process_sample(&fir, x, &y);               /* نمونه‌ای */

dsp_f32_t out[64];
dsp_fir_process_block(&fir, in, out, 64);          /* بلاکی (سریع‌تر) */
```

مثال Biquad:

```c
dsp_biquad_t bq;
dsp_biquad_init(&bq, DSP_TDF2);
dsp_biquad_set_params(&bq, DSP_BIQUAD_LPF, 48000, 1000, 0.707f, 0.0f);
/* bq اکنون یک Low-Pass 1kHz با Q=0.707 است */
```

## ۴. پیکربندی Featureها

همه‌چیز از `inc/dsp_config.h` کنترل می‌شود (یا فایل دلخواه با
`-DDSP_CONFIG_FILE="..."`). اگر ماژولی خاموش باشد، **هیچ کد، جدول یا
وابستگی‌ای از آن وارد Build نمی‌شود**:

```c
#define DSP_ENABLE_FIR              1
#define DSP_ENABLE_IIR              1
#define DSP_ENABLE_BIQUAD           1
#define DSP_ENABLE_BUTTERWORTH      1
#define DSP_ENABLE_CHEBYSHEV        0    /* خاموش → بدون کد */
#define DSP_ENABLE_ELLIPTIC         0
#define DSP_ENABLE_BESSEL           0
#define DSP_ENABLE_MEDIAN           0
#define DSP_ENABLE_MOVING_AVERAGE   1
#define DSP_ENABLE_LMS              0
#define DSP_ENABLE_RLS              0
#define DSP_ENABLE_KALMAN           0
...
```

## ۵. ساختار پروژه

```
stm32h7-dsp/
├── inc/           هدرهای عمومی (هر ماژول یک هدر)
├── src/           پیاده‌سازی‌ها (هر ماژول یک یا چند فایل)
├── tests/         تست‌های واحد + مرجع scipy
├── bench/         بنچمارک (Host + DWT برای Target)
├── examples/      ۸ مثال STM32H7
├── docs/          مستندات کامل فارسی
└── Makefile       make / make test / make bench
```

## ۶. ساخت و تست

```bash
make              # ساخت libdsp.a
make test         # ساخت و اجرای همه‌ی تست‌ها (نیازمند gcc + libm)
make bench        # بنچمارک نسبی روی Host
```

تست‌ها شامل: پاسخ ضربه/پله، پاسخ فرکانسی، پایداری، دقت عددی، سرریز، اشباع،
شرایط مرزی، خطای کمی‌سازی Fixed-Point و **اعتبارسنجی متقابل با scipy** (۱۵
کیس طراحی با تلورانس ~5e-9).

## ۷. تصمیمات مهم مهندسی (خلاصه — جزئیات در docs/Architecture)

1. **انتخاب Backend در زمان کامپایل** نه زمان اجرا — برای حذف هزینه‌ی
   dispatch از مسیر Real-Time.
2. **float64 فقط Design-Time** — پردازش نمونه‌ای در M7 با float32 (FPU).
3. **Fixed-Point IIR/Biquad فقط DF1** — حالت‌های TDF2/DF2 در Fixed به
   ذخیره‌ی state در مقیاس accumulator نیاز دارند که در 16/32 بیت نمی‌گنجد
   (همان رویه‌ی CMSIS: `arm_biquad_cascade_df1_q15`).
4. **Biquad و IIR با مرتبه‌ی بالا همیشه SOS** — فرم‌های مستقیم فقط برای
   مرتبه‌های کوچک (پایداری عددی).
5. **صفر تخصیص داینامیک** — همه‌ی بافرها توسط کاربر/استاتیک.
6. **قرارداد Bilinear T=2** — دقیقاً مطابق scipy برای اعتبارسنجی متقابل.
7. **بافرهای DMA در SRAM نه DTCM** — DTCM در STM32H7 در دسترس DMA نیست!

## ۸. اعداد کارایی (نسبی روی Host — روش دقیق در docs/Performance)

| کرنل | cyc/sample* |
|------|-------------|
| FIR 64-tap (block) | ~23 |
| FIR 64-tap symmetric | ~11 |
| Biquad cascade 8-stage | ~11 |
| FIR Q15 64-tap | ~19 |

*معادل 480MHz روی Host (x86) — روی STM32H7 با DWT اندازه‌گیری کنید.

## ۹. مسیر توسعه‌ی آینده

Kalman چندبعدی و EKF، بهینه‌سازی MVE (Cortex-M55/M85)، فیلتر CIC، تبدیل
STFT، پشتیبانی IAR/Keil برای ماکروهای حافظه — جزئیات در `docs/99_roadmap_fa.md`.
