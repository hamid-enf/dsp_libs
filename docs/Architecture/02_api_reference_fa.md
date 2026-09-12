# مرجع API — دو سطح

## ۱. API ساده (برای کاربر معمولی)

با دقت پیش‌فرض (`DSP_DEFAULT_PRECISION`) — بدون نیاز به دانش DSP:

```c
dsp_fir_t    fir;   dsp_fir_init(&fir, h, taps, bs, mode, state, stateLen);
dsp_fir_process_sample(&fir, x, &y);
dsp_fir_process_block(&fir, in, out, n);
dsp_fir_reset(&fir);

dsp_biquad_t bq;   dsp_biquad_init(&bq, DSP_TDF2);
dsp_biquad_set_params(&bq, DSP_BIQUAD_LPF, fs, fc, q, gain_db);
dsp_biquad_process_sample(&bq, x, &y);

dsp_iir_t iir;   dsp_iir_init(&iir, DSP_TDF2, order, sos, state, stateLen, 1);
dsp_iir_process_block(&iir, in, out, n);
```

## ۲. API پیشرفته (برای کاربر حرفه‌ای)

- **دقت صریح:** `dsp_fir_init_q15`, `dsp_biquad_set_params_f64`, ...
- **دسترسی به ضرایب:** `dsp_biquad_get_coeffs*`, `dsp_biquad_set_coeffs*`
- **دسترسی به حالت:** فیلدهای عمومی structها (`pState`, `head`, `d1`, `d2`, ...)
- **به‌روزرسانی Runtime:** `dsp_fir_update_coeffs*`, `dsp_iir_update_coeffs*`,
  `dsp_biquad_cascade_update_coeffs*`, `dsp_notch_set_freq*`
- **طراحی:** `dsp_design_butterworth/chebyshev1/chebyshev2/elliptic/bessel`
  با `dsp_design_params_t` + تبدیل به f32/Q15/Q31
- **Backend/Version:** `dsp_backend_get_info`, `dsp_version_string`,
  `DSP_VERSION_CHECK`
- **پاسخ فرکانسی:** `dsp_design_sos_magnitude` (برای Validation)

## ۳. نام‌گذاری

```
dsp_<ماژول>_<عمل>_<دقت>      dsp_fir_process_block_f32
dsp_<ماژول>_<عمل>            (دقت پیش‌فرض)   dsp_fir_process_block
ماکروهای کوتاه (اختیاری)     FIR_ProcessBlock  (با DSP_USE_SHORT_ALIASES)
```

## ۴. کدهای خطا

| کد | معنا |
|----|------|
| DSP_OK | موفق |
| DSP_ERR_NULL_PTR | اشاره‌گر NULL |
| DSP_ERR_INVALID_PARAMETER | پارامتر نامعتبر |
| DSP_ERR_INVALID_STATE | وضعیت داخلی نامعتبر |
| DSP_ERR_BUFFER_TOO_SMALL | بافر کاربر کوچک |
| DSP_ERR_UNSUPPORTED | حالت/دقت پشتیبانی‌نشده (مثلاً TDF2 در Fixed) |
| DSP_ERR_OVERFLOW | سرریز در Fixed |
| DSP_ERR_NOT_INITIALIZED | نمونه init نشده |
| DSP_ERR_FEATURE_DISABLED | ماژول خاموش در config |
| DSP_ERR_NO_MEMORY | تخصیص داینامیک ناموفق (فقط Optional) |
| DSP_ERR_DESIGN_CONVERGENCE | طراحی همگرا نشد |

## ۵. Versioning

```c
DSP_VERSION_MAJOR / MINOR / PATCH   →  1.0.0
DSP_VERSION_CHECK(major, minor, patch)   /* برای سازگاری API */
```

سیاست: تا 1.x، افزودن API با Minor، تغییر مخرب با Major.
