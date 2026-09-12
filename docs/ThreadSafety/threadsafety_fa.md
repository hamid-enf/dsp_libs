# Thread Safety و استفاده در Real-Time

## ۱. مدل اجرا

کتابخانه **بدون Lock** در مسیر پردازش است. مدل توصیه‌شده:

```
پیکربندی (Init/Design)   →   فقط یک‌بار، قبل از شروع پردازش
پردازش (Process)          →   Task/ISR اختصاصی، Reentrant
به‌روزرسانی ضرایب          →   بین دو بلاک، از Context غیر Real-Time
```

## ۲. جدول ایمنی

| دسته | مثال | ISR-safe | هم‌زمان با Process |
|------|------|----------|-------------------|
| Init | `dsp_fir_init*` | خیر (در ISR نکنید) | خیر |
| Config | `dsp_biquad_set_params*` | خیر | خیر (بین بلاک‌ها) |
| Process | `dsp_fir_process_block*` | **بله** | بله (نمونه‌های مجزا) |
| Update | `dsp_fir_update_coeffs*` | بله (اتمیک) | بین بلاک‌ها |
| Reset | `dsp_fir_reset*` | بله | بین بلاک‌ها |
| Design | `dsp_design_*` | خیر | خیر |

## ۳. FreeRTOS

- پردازش را در یک Task با اولویت بالا (یا ISR) قرار دهید.
- برای اشتراک داده بین Taskها از Queue/StreamBuffer استفاده کنید؛ بافرهای
  کتابخانه را بین Taskها share نکنید مگر با mutex در سطح شما.
- `configASSERT` روی پارامترها اختیاری است (کتابخانه خودش dsp_err_t برمی‌گرداند).

## ۴. الگوی Double-Buffer برای به‌روزرسانی بدون وقفه

```c
/* Context 1 (Real-Time): */
for (;;) {
    sem_wait(&dma_done);
    dsp_fir_process_block_f32(&fir, in, out, N);
}
/* Context 2 (کنترل/پیکربندی): */
dsp_fir_update_coeffs_f32(&fir, newCoeffs, taps);   /* اتمیک — بین بلاک‌ها */
```

تغییر ضرایب فقط بین دو `process_block` مؤثر است؛ اگر بلاک‌ها طولانی‌اند
(> چند ms)، یا بلاک را کوچک کنید یا از الگوی ping-pong در سطح خودتان
استفاده کنید.
