# Performance — روش اندازه‌گیری و اعداد

## ۱. روش اندازه‌گیری روی STM32H7 (دقیق)

1. **DWT Cycle Counter** را فعال کنید:
   ```c
   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
   DWT->CYCCNT = 0;
   DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
   ```
2. قبل از اندازه‌گیری: Cache را warm کنید (چند اجرای اول)، interrupts را
   موقتاً غیرفعال کنید، و از بافرهای DTCM استفاده کنید (بدون کش).
3. اندازه بگیرید: `t0 = DWT->CYCCNT; process(...); t1 = DWT->CYCCNT;`
   و به ازای هر نمونه تقسیم کنید.

`make bench` روی Host اعداد نسبی می‌دهد (x86)؛ روی Target با
`-DDSP_TARGET_STM32H7` از DWT استفاده می‌شود.

## ۲. تخمین سیکل‌ها در M7 (راهنمای تحلیل)

| عملیات | سیکل تقریبی (M7, FPU) |
|--------|------------------------|
| FMA (fused multiply-add) | 1 |
| ضرب-جمع int32 | 1 |
| ضرب int64 → جمع | ~3-4 |
| فراخوانی تابع | 4-6 |
| دسترسی DTCM | 1 |
| دسترسی AXI-SRAM (کش‌خورده) | 1 (هیت) تا ~20 (میس) |
| saturating shift + sat | ~2-3 |

بنابراین:
- FIR f32 با N tap و Block: ≈ N+2 سیکل/نمونه (FMA + حلقه).
- FIR q15 با N tap: ≈ 2N سیکل/نمونه (ضرب int32 + جمع) — قابل بهبود با
  SIMD (مثلاً SMLAD) در نسخه‌های بعدی.
- Biquad TDF2 f32: ≈ 10-14 سیکل/نمونه.

## ۳. اعداد مرجع (Host — فقط نسبی)

| کرنل | cyc/sample معادل* |
|------|-------------------|
| FIR 64-tap block f32 | ~23 |
| FIR 64-tap sample f32 | ~30 |
| FIR 64-tap symmetric f32 | ~11 |
| Biquad cascade 8×TDF2 f32 | ~11 |
| FIR 64-tap q15 | ~19 |

*با فرض 480MHz روی x86 — **روی M7 با DWT اندازه‌گیری کنید** (اعداد M7
ممکن است کاملاً متفاوت باشند؛ FMA و Pipeline تفاوت ایجاد می‌کند).

## ۴. حداکثر نرخ نمونه‌برداری نظری

$$\text{maxSR} = \frac{f_{CPU}}{\text{cycles/sample}}$$

مثال: f32 FIR 64-tap با ~70 سیکل/نمونه واقعی روی M7@480MHz →
maxSR ≈ 6.8MS/s — بسیار فراتر از نیازهای صوتی/حسگر.

## ۵. نکات بهینه‌سازی برای M7

1. **Block Processing** به‌جای Sample (حلقه‌ی باز، prefetch بهتر).
2. `-O3 -mfpu=fpv5-d16 -mfloat-abi=hard -ffast-math` (اگر دقت اجازه دهد).
3. بافرها در **DTCM**، ضرایب `const` در Flash (ITCM برای کد بحرانی).
4. الاین‌کردن ۸/۱۶-بایتی بافرها (`DSP_ALIGN32`).
5. `-ffunction-sections -Wl,--gc-sections` برای حذف ماژول‌های استفاده‌نشده.
6. CMSIS-DSP Backend را A/B کنید: `DSP_USE_CMSIS_DSP=1` — برای FIR
   arm_fir_f32 و برای Biquad arm_biquad_cascade_df1/df2T بهینه‌اند.

## ۶. Trade-offها (خلاصه)

| انتخاب | Performance | Accuracy | RAM | Flash |
|--------|-------------|----------|-----|-------|
| f32 | ★★★ | ★★ | 4B/نمونه | کم |
| q15 | ★★ (SIMD بعدی) | ★ | 2B/نمونه | کم |
| q31 | ★★ | ★★★ | 4B/نمونه | کم |
| f64 | ★ (نرم‌افزاری) | ★★★★ | 8B/نمونه | زیاد |
| CMSIS backend | ★★★ (بعضی) | = | = | + |

## ۷. Flash/RAM Footprint (راهنما)

با `--gc-sections` و فقط FIR+BIQUAD فعال: کتابخانه ≈ 1-3KB Flash (بسته به
دقت‌های فعال) + RAM فقط بافرهای کاربر. اندازه‌ی دقیق را با `arm-none-eabi-size`
بعد از لینک بررسی کنید.
