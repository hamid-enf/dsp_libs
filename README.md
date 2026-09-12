# STM32H7 DSP Filter Library — v1.0.0

کتابخانه‌ی جامع پردازش سیگنال دیجیتال برای **STM32H7 (Cortex-M7)**:
ماژولار، Production-Ready، با Compile-Time Feature Selection، چهار دقت
(f32/f64/Q15/Q31)، دو Backend (داخلی / CMSIS-DSP)، صفر تخصیص داینامیک و
مستندات کامل فارسی.

## مستندات فارسی (شروع از اینجا)

- **[README_FA.md](docs/README_FA.md)** — نمای کلی، شروع سریع، پیکربندی
- **[نقد معماری](docs/Architecture/00_architecture_review_fa.md)** —
  تصمیمات و Trade-offهای مهندسی (مطابق خواسته‌ی پروژه)
- **[معماری](docs/Architecture/01_architecture_fa.md)** — لایه‌ها، حافظه، Thread Safety
- **[مرجع API](docs/Architecture/02_api_reference_fa.md)**
- مستندات ماژول‌ها: [FIR](docs/FIR/) [IIR](docs/IIR/) [Biquad](docs/Biquad/)
  [Butterworth](docs/Butterworth/) [Chebyshev](docs/Chebyshev/)
  [Elliptic](docs/Elliptic/) [Bessel](docs/Bessel/)
  [MovingAverage](docs/MovingAverage/) [Median](docs/Median/)
  [SavitzkyGolay](docs/SavitzkyGolay/) [DCBlocker](docs/DCBlocker/)
  [Notch](docs/Notch/) [Adaptive](docs/Adaptive/) [Kalman](docs/Kalman/)
  [Resampling](docs/Resampling/) [Utilities](docs/Utilities/)
  [Performance](docs/Performance/) [ThreadSafety](docs/ThreadSafety/)

## ساخت و تست

```bash
make              # libdsp.a
make test         # تست‌ها (شامل اعتبارسنجی scipy)
make bench        # بنچمارک
```

## ساختار

```
inc/     هدرهای عمومی (dsp.h، dsp_config.h و...)
src/     پیاده‌سازی‌ها (هر ماژول Feature-guarded)
tests/   تست‌ها + مراجع scipy
bench/   بنچمارک (Host/DWT)
examples/  ۸ مثال STM32H7 (ADC+DMA، I2S، حسگر، LMS، Kalman و...)
docs/    مستندات کامل فارسی
```

## حداقل استفاده

```c
#include "dsp.h"
static const dsp_f32_t h[16] = { /* ضرایب FIR */ };
static dsp_f32_t state[16 + 64 - 1];
dsp_fir_t fir;
dsp_fir_init(&fir, h, 16, 64, DSP_FORM_DIRECT, state, sizeof(state)/4);
dsp_fir_process_block(&fir, in, out, 64);
```

## License
MIT (فایل LICENSE را ببینید).
