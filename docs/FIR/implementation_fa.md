# FIR — پیاده‌سازی

## ساختار نمونه

```c
typedef struct {
    uint16_t        taps;      /* تعداد ضرایب */
    uint16_t        blockSize; /* حداکثر اندازه‌ی بلاک */
    uint8_t         mode;      /* DIRECT / TRANSPOSED / SYMMETRIC */
    const dsp_f32_t *pCoeffs;  /* ضرایب (const → می‌تواند در Flash باشد) */
    dsp_f32_t       *pState;   /* بافر حالت (کاربر) */
    uint16_t        head;      /* ایندکس حلقه برای Sample */
    uint32_t        stateLen;
} dsp_fir_f32_t;               /* + نمونه‌های f64/q15/q31 */
```

## حالت‌ها
- **DIRECT:** بافر خطی `taps+blockSize-1` با Append+Rewind (سازگار با DMA،
  بدون modulo در حلقه‌ی بلاک). Sample با حلقه‌ی حلقوی.
- **TRANSPOSED:** بافر `taps`؛ برای Sample های پشت‌سرهم سریع و بدون کپی.
- **SYMMETRIC:** بهره‌برداری از تقارن — نصف ضرب‌ها.

## دقت‌ها
- f32: accumulator float (FPU+FMA روی M7).
- q15: accumulator int32، خروجی `sat((acc + 2^14) >> 15)`. با
  `DSP_FIXED_ACC_SATURATE=1` جمع اشباعی هر مرحله (ایمن‌تر برای ورودی‌های
  بزرگ).
- q31: accumulator int64، خروجی `sat((acc + 2^30) >> 31)`.

## به‌روزرسانی ضرایب در زمان اجرا
`dsp_fir_update_coeffs_f32(p, newCoefs, n)` اشاره‌گر را اتمیک عوض می‌کند
(بدون کپی). **الگوی Double-Buffer پیشنهادی:**

```c
static const dsp_f32_t *active = coefsA;
/* در Context غیر Real-Time: */
dsp_fir_update_coeffs_f32(&fir, coefsB, taps);  /* بین دو بلاک */
active = coefsB;   /* نسخه‌ی جدید برای خواندن */
/* در مسیر Real-Time فقط process_block اجرا می‌شود؛ هیچ تغییری داده نمی‌شود */
```

## نکات M7
- `-O3 -mfpu=fpv5-d16 -mfloat-abi=hard` برای استفاده از FMA.
- ضرایب را `const` در Flash نگه دارید (حفظ RAM).
- بافر state را با `DSP_PLACE_DTCM` بگذارید (فقط CPU).
- برای Block های بزرگ، `pCoeffs` و `pState` را ۴-بایت (ترجیحاً ۸/۱۶-بایت)
  الاین کنید.

## مصرف حافظه (مثال)
FIR 48-tap، بلاک 128: coeffs 192B (Flash)، state 700B (RAM).
