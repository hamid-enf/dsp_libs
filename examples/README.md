# مثال‌های STM32H7

| # | مثال | ماژول‌ها | جریان |
|---|------|----------|-------|
| 1 | ADC + DMA + FIR | `dsp_fir` | ADC 1kHz → FIR LP 100Hz |
| 2 | ADC + DMA + IIR | `dsp_design_butterworth`, `dsp_iir` | طراحی Runtime + SOS |
| 3 | ADC + Biquad + FFT | `dsp_dcblock`, `dsp_biquad`, CMSIS-DSP FFT | BP + طیف |
| 4 | Audio I2S + Biquad | `dsp_biquad` (3-band EQ) | I2S 48kHz Stereo |
| 5 | Sensor + Moving Average | `dsp_sma` | O(1) نرم‌سازی حسگر |
| 6 | Sensor + Median | `dsp_median` (Q15) | حذف Spike |
| 7 | Noise Cancellation + LMS | `dsp_lms` | دو میکروفون |
| 8 | Sensor Fusion + Kalman | `dsp_kalman1d` | ترکیب دو حسگر |

ساختار هر مثال: یک `main.c` مستقل با HAL — بخش‌های `SystemClock_Config` و
`MX_..._Init` طبق پروژه‌ی CubeMX شما اضافه می‌شود (جاهای مشخص‌شده با کامنت).

برای مثال‌های header-only و آماده‌ی استفاده در پروژه‌ی صوتی، فایل فارسی
[`README_FA.md`](README_FA.md) و پوشه‌ی [`filters/`](filters/) را ببینید.
فایل [`realtime_audio_callback.h`](realtime_audio_callback.h) یک زنجیره‌ی
واقعی PCM16 (DC Blocker → Notch → Low-Pass → Limiter) را نشان می‌دهد:
فیلتر در Init ساخته می‌شود و callback فقط state را جلو می‌برد.

نکات مشترک:
- بافرهای DMA → SRAM (نه DTCM)؛ بافرهای CPU → DTCM برای سرعت.
- روی AXI-SRAM، برای DMA به Cache Maintenance نیاز است (`dsp_cache_*`).
- برای Max Performance از Block Processing و `-O3 -mfpu=fpv5-d16` استفاده کنید.
