# فیلترهای تطبیقی — LMS / NLMS / RLS

## ۱. معرفی
فیلترهایی که ضرایب خود را **بر اساس سیگنال** به‌روزرسانی می‌کنند تا خطای
بین خروجی و سیگنال مرجع را کمینه کنند.

## ۲. کاربرد
- حذف نویز (Noise Cancellation — مثال ۷)
- حذف اکو (Echo Cancellation)
- شناسایی سیستم (System Identification)
- فیلتر تطبیقی حسگر (حذف Drift همبسته با مرجع)

## ۳. مزایا
- بدون دانش قبلی از نویز (فقط همبستگی با مرجع)
- سازگاری با تغییرات محیط

## ۴. معایب
- نیاز به سیگنال مرجع باکیفیت
- پارامترهای tuning حساس (μ، λ)
- فقط float32 (Fixed نامناسب است — به نقد معماری مراجعه کنید)

## ۵. محدودیت‌ها
- LMS برای ورودی‌های رنگی (Correlated) کند همگرا می‌شود → NLMS/RLS
- RLS گران است: RAM = len²

## ۶. مدل ریاضی

**LMS:** با بردار ورودی $\mathbf{x}[n]$، وزن $\mathbf{w}[n]$:
$$y[n] = \mathbf{w}^T[n]\,\mathbf{x}[n], \qquad e[n] = d[n] - y[n]$$
$$\mathbf{w}[n+1] = (1-\mu\gamma)\,\mathbf{w}[n] + \mu\, e[n]\,\mathbf{x}[n]$$

**NLMS:** گام نرمال‌شده:
$$\mu_{eff} = \frac{\mu}{\|\mathbf{x}[n]\|^2 + \delta}$$

**RLS:** با ماتریس کوواریانس $\mathbf{P}$ و ضریب فراموشی $\lambda$:
$$\mathbf{k}[n] = \frac{\mathbf{P}[n-1]\,\mathbf{x}[n]}
{\lambda + \mathbf{x}^T[n]\,\mathbf{P}[n-1]\,\mathbf{x}[n]}$$
$$\mathbf{w}[n] = \mathbf{w}[n-1] + \mathbf{k}[n]\,e[n]$$
$$\mathbf{P}[n] = \frac{\mathbf{P}[n-1] - \mathbf{k}[n]\,\mathbf{x}^T[n]\mathbf{P}[n-1]}{\lambda}$$

## ۷–۸. متغیرها
$d$: سیگنال مطلوب، $e$: خطا، $\mu$: گام یادگیری، $\gamma$: Leakage،
$\delta$: ε پایداری، $\lambda$: ضریب فراموشی (0.9..1)، $\mathbf{P}$: کوواریانس.

## ۹. انتخاب پارامترها
- $\mu < 1/\lambda_{max}$ (LMS) — معمولاً 0.001..0.05 برای len=16..64.
- NLMS: μ ~ 0.1..0.5 (مستقل از توان ورودی).
- RLS: λ نزدیک 1 (0.99) برای همگرایی پایدار؛ δ = واریانس اولیه‌ی P.

## ۱۰–۱۴. اثرها
- μ بزرگ → همگرایی سریع + نویز وزن زیاد/واگرایی.
- Leakage → پایداری در برابر ورودی‌های صفر، ولی Bias.
- RLS همگرایی ~۱۰× سریع‌تر از LMS با هزینه‌ی O(len²).
- CPU: LMS/NLMS O(len)؛ RLS O(len²). RAM: 2len و len²+2len.

## ۱۷–۲۲. مثال‌ها
مثال ۷ (حذف نویز دو میکروفون). شناسایی سیستم:

```c
dsp_lms_f32_t lms;
dsp_lms_init_f32(&lms, 64, 0.005f, 0.0f, w, xl);
dsp_lms_process_f32(&lms, x, d, &y, &e);   /* e = خطا، y = تخمین */
```
