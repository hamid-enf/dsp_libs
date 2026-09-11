# Chebyshev I / II — نظریه و طراحی

## ۱. معرفی
**Type I:** موج‌داری برابر (Equiripple) در پاس‌باند — گذار تندتر از
Butterworth برای همان مرتبه. **Type II (Inverse):** موج‌داری در استاپ‌باند،
پاس‌باند یکنواخت، با صفرهای موهومی.

## ۲. کاربرد
وقتی ripple پاس‌باند مجاز است و مرتبه مهم است (فیلترهای مخابراتی،
گزینش‌پذیری بالا).

## ۳. مزایا
- مرتبه‌ی کمتر از Butterworth برای مشخصات یکسان
- Type II: بدون ripple در پاس‌باند

## ۴. معایب
- Type I: ripple پاس‌باند (مثلاً ±1dB)
- Type II: تضعیف استاپ‌باند فقط تا سطح مشخص (بدون رفتن به صفر مطلق در LP)

## ۵. محدودیت‌ها
- نوع I برای سیگنال‌هایی که ripple مهم است مناسب نیست

## ۶. مدل ریاضی (Type I)
$$|H(j\omega)|^2 = \frac{1}{1 + \varepsilon^2 T_N^2(\omega/\omega_c)}, \qquad
\varepsilon = \sqrt{10^{R_p/10}-1}$$

که $T_N$ چندجمله‌ای چبیشف نوع اول است.

## ۷. فرمول قطب‌ها
با $a = \dfrac{1}{N}\operatorname{asinh}\dfrac{1}{\varepsilon}$ و
$\theta_k = \dfrac{\pi(2k+1)}{2N}$:

$$p_k = -\sinh(a)\sin\theta_k + j\cosh(a)\cos\theta_k$$

گین DC: فرد → 1، زوج → $10^{-R_p/20}$.

**Type II:** با $d_e = \dfrac{1}{\sqrt{10^{R_s/10}-1}}$،
$\mu = \dfrac{1}{N}\operatorname{asinh}\dfrac{1}{d_e}$:

- صفرها: $z_k = \dfrac{j}{\sin\left(\frac{m\pi}{2N}\right)}$ (m فرد)
- قطب‌ها: $p_k = -\dfrac{1}{\sinh(\mu + j\theta_k)}$

## ۸. تعریف متغیرها
$\varepsilon$: ضریب ripple، $R_p$: ripple پاس‌باند (dB)، $R_s$: تضعیف
استاپ‌باند (dB)، $N$: مرتبه.

## ۹. انتخاب پارامترها
- $R_p$ کوچک (0.1..1dB) برای کاربردهای دقیق؛ $R_s$ ≥ 40dB معمول.
- تخمین مرتبه: $N \ge \dfrac{\cosh^{-1}\sqrt{(10^{A_s/10}-1)/(10^{A_p/10}-1)}}
  {\cosh^{-1}(\Omega_s/\Omega_p)}$

## ۱۰. اثر تغییر پارامترها
افزایش $R_p$ → گذار تندتر + ripple بیشتر. افزایش $N$ → گذار تندتر.

## ۱۱. اثر روی Phase
Type I: انحراف فاز بزرگ‌تر از Butterworth (نزدیک لبه). Type II: بهتر.

## ۱۲. اثر روی Magnitude
Type I: ripple پاس‌باند ±Rp. Type II: یکنواخت تا لبه، سپس تضعیف ≥ Rs.

## ۱۳. اثر روی Noise
ریپل پاس‌باند گین را نوسانی می‌کند؛ Type II پاس‌باند تمیزتری دارد.

## ۱۴. اثر روی Stability
مانند Butterworth؛ با N بزرگ مراقب کمی‌سازی Fixed باشید (SOS).

## ۱۵–۱۶. CPU/RAM
همان Biquad cascade — N/2 طبقه.

## ۱۷–۲۲. مثال‌ها
همان الگوی `dsp_design_chebyshev1/2(&cfg)` با `cfg.ripple_db`/`cfg.stop_db`.
