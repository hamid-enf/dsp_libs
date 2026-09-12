# Resampling — Decimation / Interpolation

## ۱. معرفی
تغییر نرخ نمونه‌برداری با فیلتر Low-Pass پروتوتایپ و ساختار Polyphase —
بدون محاسبه‌ی نمونه‌های اضافی (کارایی D× یا L×).

## ۲. کاربرد
- کاهش نرخ ADC پرسرعت قبل از پردازش (Decim)
- افزایش نرخ برای DAC/بازتولید (Interp)
- تبدیل نرخ بین سیستم‌ها (مثلاً 48k ↔ 44.1k با زنجیره)

## ۳. مزایا
- بهره‌ی Polyphase: برای Decim فقط taps MAC به ازای هر خروجی (نه taps×D)
- ضد-آلیاسینگ/ضد-تصویر داخلی با پروتوتایپ LP

## ۴. معایب
- نیاز به طراحی پروتوتایپ خوب (پنجره‌ی sinc)
- تأخیر فیلتر

## ۵. محدودیت‌ها
- فقط نرخ‌های صحیح L/D؛ برای نرخ‌های گنگ، زنجیره یا رویکرد چندمرحله‌ای

## ۶. مدل ریاضی
**Decim (ضریب D):**
$$y[m] = \sum_{k=0}^{N-1} h[k]\, x[Dm - k] \qquad (N = D\cdot P)$$

**Interp (ضریب L):** با $p = m \bmod L$ و $n_0 = \lfloor m/L \rfloor$:
$$y[m] = \sum_{j=0}^{P-1} h[p + jL]\, x[n_0 - j]$$

## ۷–۸. متغیرها
$h$: پروتوتایپ (طول N)، $P$: ضرب‌ها به ازای هر فاز، $L/D$: نسبت‌ها.

## ۹. انتخاب پارامترها
- پروتوتایپ: sinc پنجره‌دار با cutoff = 1/max(L,D) و P=8..32.
- `dsp_resample_design_lp_f32(h, taps, cutoff)` برای طراحی سریع.

## ۱۰–۱۴. اثرها
- P بزرگ → گذار تندتر، RAM/CPU بیشتر.
- آلیاسینگ با cutoff مناسب حذف می‌شود.
- فاز: خطی (پروتوتایپ متقارن).
- CPU: Decim = taps/output؛ Interp = P/output.
- RAM: Decim = N؛ Interp = P.

## ۱۷–۲۲. مثال‌ها
```c
dsp_f32_t proto[32];
dsp_resample_design_lp_f32(proto, 32, 0.25f);   /* پروتوتایپ ÷4 */
dsp_decim_f32_t dc;
dsp_decim_init_f32(&dc, 4, 8, proto, state);
dsp_decim_process_f32(&dc, in, out, nOut);       /* consumes 4*nOut */
/* Interp مشابه: dsp_interp_init_f32(&ip, 4, 8, proto, st); */
```
ADC: Decim بعد از نمونه‌برداری پرسرعت؛ Audio: Interp قبل از DAC.
