# بلوک‌های پردازش عمومی — Gain/RMS/Peak/Envelope/Limiter

## معرفی و مدل ریاضی

| بلوک | فرمول | کاربرد |
|------|-------|--------|
| Gain | $y = g\,x$ (با اشباع اختیاری) | تنظیم سطح |
| Normalize | $y = x/\max\|x\|$ | نرمال‌سازی به ±1 |
| Saturate/Clip | محدود به [lo,hi] | حذف بیشینه |
| RMS | $\sqrt{\tfrac{1}{N}\sum x^2}$ | سطح توان |
| Peak | حمله/رهاسازی نمایی | Level-meter |
| Envelope | یک‌قطبی حمله/رهاسازی | ردیابی پوش |
| Limiter | کاهش گین بالای آستانه | محافظت خروجی |

**Limiter:** اگر $|x| > T$ گین هدف = $\min(g, C/|x|)$ و گین با ضریب حمله به
سمت آن حرکت می‌کند؛ زیر آستانه با ضریب رهاسازی به 1 بازمی‌گردد.

## نکات پیاده‌سازی
- همه O(1) یا O(N)؛ بدون تخصیص.
- Peak/Envelope/Limiter **نمونه‌ای** و مناسب مسیر Real-Time.
- `dsp_limiter_process_block_f32` برای بلاک.

## مثال
```c
dsp_env_f32_t env; dsp_env_init_f32(&env, 0.01f, 0.1f);
dsp_env_process_f32(&env, x, &level);      /* پوش سیگنال */
dsp_limiter_f32_t lim;
dsp_limiter_init_f32(&lim, 0.8f, 0.95f, 0.05f, 0.01f);
dsp_limiter_process_f32(&lim, x, &y);      /* خروجی محافظت‌شده */
```
