# معماری کتابخانه

## ۱. نمای لایه‌ای

```
                    Application
                         |
                    Public API  (dsp.h + ماژول‌ها)
                         |
                 Filter Abstraction  (ساختار نمونه‌ها + قراردادها)
                         |
              +----------+----------+
              |                     |
         Filter Algorithms     Utility Blocks
              |                     |
   +----------+----------+----------------+
   |          |          |                |
  FIR        IIR       Biquad        Adaptive/...
   |          |          |                |
   +----------+----------+----------------+
                         |
                   DSP Backend   (انتخاب کامپایل‌زمانی)
                    /         \
               Custom        CMSIS-DSP
                  |
            Cortex-M7 Optimized
                  |
          FPU / SIMD / DSP ISA
```

## ۲. گراف وابستگی ماژول‌ها

```
dsp.h
 ├── dsp_config.h          (هیچ وابستگی)
 ├── dsp_port.h            (dsp_config.h؛ انواع، اشباع، چیدمان حافظه)
 ├── dsp_types.h           (dsp_port.h؛ خطاها، نسخه، شمارنده‌ها)
 ├── dsp_fir.h / dsp_iir.h / dsp_biquad.h   (dsp_types.h)
 │     └── پیاده‌سازی‌ها از تمپلیت‌ها (src/dsp_*_tpl.h) با ۴ دقت
 ├── dsp_design.h          (dsp_types.h)
 │     └── dsp_math.h/.c   (اعداد مختلط، توابع بیضوی، ریشه‌یابی)
 ├── dsp_moving.h / dsp_median.h / dsp_savgol.h / dsp_dcblock.h / dsp_notch.h
 ├── dsp_adaptive.h / dsp_kalman.h / dsp_resample.h / dsp_utils.h
 └── dsp_backend.h         (انتخاب Backend + Cache hooks)
```

**قانون:** هیچ ماژولی به ماژول دیگر وابستگی سخت ندارد (مثلاً IIR در حالت
SOS موتور خودش را دارد و به Biquad وابسته نیست) تا خاموش‌کردن هر Feature
هیچ اثر جانبی نداشته باشد.

## ۳. Feature Configuration — جریان Build

1. `dsp_config.h` (یا `DSP_CONFIG_FILE`) ماکروهای `DSP_ENABLE_*` و
   `DSP_USE_*` را تعیین می‌کند.
2. هر فایل `src/*.c` با `#if DSP_ENABLE_X` شروع می‌شود؛ اگر خاموش باشد فایل
   عملاً خالی است (هیچ Flash/RAM/جدولی).
3. `inc/dsp.h` فقط هدرهای فعال را include می‌کند.
4. حتی در حالت «همه روشن»، کامپایلر فقط توابع استفاده‌شده را نگه می‌دارد
   (با `-ffunction-sections -Wl,--gc-sections` می‌توان Footprint را به حداقل
   رساند).

## ۴. Precision — معماری تمپلیت

FIR/IIR/Biquad با یک تمپلیت C (src/dsp_*_tpl.h) برای چهار دقت نمونه‌سازی
می‌شوند:

| دقت | نوع نمونه | Accumulator | خروجی |
|-----|-----------|-------------|-------|
| f32 | float | float | float |
| f64 | double | double | double |
| q15 | int16_t | int32_t | sat(acc >> shift) |
| q31 | int32_t | int64_t | sat(acc >> shift) |

API ساده (`dsp_fir_t`, `dsp_fir_init` و...) به دقت پیش‌فرض
(`DSP_DEFAULT_PRECISION`) نگاشت می‌شود؛ API پیشرفته با پسوند دقت
(`dsp_fir_init_q15`) در دسترس است.

## ۵. مدل حافظه (Zero Dynamic Allocation)

- همه‌ی ساختارها و بافرها توسط **کاربر** تخصیص می‌یابند (استاتیک یا در
  استک/هیپ کاربر).
- هیچ `malloc/calloc/realloc/free` در مسیر پیش‌فرض وجود ندارد.
- `DSP_USE_DYNAMIC_ALLOCATION = 1` فقط برای پلتفرم‌های میزبان (در نسخه‌های
  آتی برخی ساختارها را خودکار تخصیص می‌دهد).

**اندازه‌های بافر موردنیاز (مستند در هر هدر):**

| ماژول | بافر حالت |
|-------|-----------|
| FIR Direct/Sym | taps + blockSize − 1 |
| FIR Transposed | taps |
| Biquad DF1 | ۴ × (نمونه) داخل struct |
| Biquad TDF2 | ۲ × داخل struct |
| Cascade TDF2 | ۲ × stages |
| Cascade DF1 | ۴ × stages |
| IIR مستقیم | ۲N (DF1) یا N (DF2/TDF2) |
| SMA | window |
| Median | ۲ × window |
| RLS | len + len + len² |

## ۶. چیدمان حافظه در STM32H7

```c
DSP_PLACE_DTCM     /* بافرهای CPU-only: سریع‌ترین */
DSP_PLACE_SRAM1/2  /* بافرهای DMA (DTCM در دسترس DMA نیست!) */
DSP_PLACE_AXISRAM  /* بزرگ، کش‌دار → برای DMA نیاز به dsp_cache_* */
DSP_PLACE_ITCM     /* کد بحرانی */
```

ماکروهای معادل برای IAR/Keil را می‌توان در `dsp_port.h` اضافه کرد
(بخش «Memory Placement»).

## ۷. Thread Safety

| دسته | توابع | ایمنی |
|------|-------|-------|
| Init/Config | `*_init*`, `*_set_params*`, `*_set_coeffs*` | **فقط قبل از شروع پردازش**؛ نه هم‌زمان با Processing |
| Processing | `*_process_sample*`, `*_process_block*` | **Reentrant**؛ هر نمونه فقط state خودش را لمس می‌کند؛ قابل‌اجرا در ISR |
| Update | `*_update_coeffs*` | بین دو بلاک، از یک Context؛ نه در حین process_block |
| Design | `dsp_design_*` | فقط در Init؛ نه در ISR (چند ms برای Elliptic) |
| Reset | `*_reset*` | بین بلاک‌ها؛ نه در حین process_block همان نمونه |

هیچ Lock سنگینی به مسیر Real-Time اضافه نشده است؛ در FreeRTOS کافی است
Processing در یک Task/ISR اختصاصی باشد.

## ۸. خطایابی (Error Handling)

همه‌ی توابع عمومی `dsp_err_t` برمی‌گردانند (`DSP_OK=0`، خطاها مثبت).
هرگز در مسیر پردازش خطای نرم‌افزاری تولید نمی‌شود (پارامترها در Init
اعتبارسنجی می‌شوند) — یعنی حلقه‌ی Real-Time می‌تواند بدون چک خطا اجرا شود
و فقط Init/Config خطا برمی‌گرداند.
