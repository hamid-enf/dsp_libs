/**
 ******************************************************************************
 * @file    dsp_config.h
 * @brief   پیکربندی سراسری کتابخانه DSP برای STM32H7 (Cortex-M7)
 *
 * این فایل تنها نقطه‌ی کنترل Featureها و Precision ها و Backend کتابخانه است.
 *
 * دو روش استفاده:
 *   1) ویرایش مستقیم همین فایل.
 *   2) تعریف یک فایل پیکربندی جداگانه و معرفی آن هنگام کامپایل:
 *        gcc -DDSP_CONFIG_FILE=\"my_config.h\" ...
 *
 * قانون طلایی: اگر یک Feature خاموش باشد (0)، هیچ کد، جدول یا وابستگی مربوط
 * به آن وارد Build نمی‌شود (همه‌ی ماژول‌ها با #if بر اساس همین ماکروها محافظت
 * می‌شوند).
 *
 * @note  نسخه‌ی 1.0.0 — معماری به صورت Compile-Time Feature Selection است.
 ******************************************************************************
 */

#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H

/*
 * شناسه‌های عددی برای استفاده در #ifهای preprocessor. نام‌های DSP_PREC_*
 * خودشان enum هستند و preprocessor مقدار enum را نمی‌داند؛ استفاده‌ی مستقیم
 * از آن نام‌ها در #if همیشه به صفر تبدیل می‌شود و انتخاب Q15/Q31 را خراب می‌کند.
 */
#define DSP_PRECISION_ID_F32  0
#define DSP_PRECISION_ID_Q15  1
#define DSP_PRECISION_ID_Q31  2
#define DSP_PRECISION_ID_F64  3

/* ---------------------------------------------------------------------------
 * پشتیبانی از فایل پیکربندی خارجی
 * ------------------------------------------------------------------------- */
#ifdef DSP_CONFIG_FILE
  #include DSP_CONFIG_FILE
  /* اگر کاربر فایل خودش را داده، بقیه‌ی این فایل نادیده گرفته شود؛
     کاربر مسئول تعریف تمام ماکروهاست. */
#else

/* ===========================================================================
 * 1) FEATURE SWITCHES — کنترل وجود/عدم وجود هر ماژول در Build
 * =========================================================================== */

/* --- هسته‌ی فیلترها --- */
#define DSP_ENABLE_FIR                 1   /* FIR: direct / transposed / symmetric  */
#define DSP_ENABLE_IIR                 1   /* IIR: DF1 / DF2 / TDF2 + SOS cascade    */
#define DSP_ENABLE_BIQUAD              1   /* Biquad single + cascade (SOS)          */

/* --- طراحی فیلتر (تولید خودکار ضرایب) --- */
#define DSP_ENABLE_BUTTERWORTH         1
#define DSP_ENABLE_CHEBYSHEV           1   /* Type I و Type II */
#define DSP_ENABLE_ELLIPTIC            1   /* Cauer — نیازمند توابع بیضوی (گران‌تر) */
#define DSP_ENABLE_BESSEL              1   /* برای Group-Delay بحرانی               */

/* --- فیلترهای کاربردی --- */
#define DSP_ENABLE_MOVING_AVERAGE      1   /* SMA / WMA / EMA */
#define DSP_ENABLE_MEDIAN              1
#define DSP_ENABLE_SAVGOL              1   /* Savitzky-Golay */
#define DSP_ENABLE_DCBLOCK             1   /* حذف DC */
#define DSP_ENABLE_NOTCH               1   /* ناچ فیلتر (مثلاً 50/60 هرتز) */

/* --- فیلترهای تطبیقی (کاملاً Optional) --- */
#define DSP_ENABLE_LMS                 1
#define DSP_ENABLE_NLMS                1
#define DSP_ENABLE_RLS                 0   /* گران‌ترین الگوریتم: RAM = len*len */

/* --- تخمین‌گر --- */
#define DSP_ENABLE_KALMAN              1   /* Kalman 1D (قابل توسعه به چندبعدی) */

/* --- تغییر نرخ نمونه‌برداری --- */
#define DSP_ENABLE_RESAMPLING          1   /* Decimation / Interpolation / SRC */

/* --- بلوک‌های پردازشی عمومی --- */
#define DSP_ENABLE_SIGNAL_UTILS        1   /* Gain/RMS/Peak/Envelope/Limiter/Saturate */

/* ===========================================================================
 * 2) PRECISION SELECTION — کدام دقت‌ها وارد Build شوند؟
 * ===========================================================================
 * حداقل یکی باید فعال باشد. برای STM32H7 مسیر اصلی Performance، float32 است
 * (FPU تک‌دقتی). float64 در M7 نرم‌افزاری است و فقط برای طراحی ضرایب
 * (Design-Time) توصیه می‌شود، نه پردازش نمونه‌ای.
 */
#define DSP_USE_FLOAT32                1
#define DSP_USE_FLOAT64                0   /* پردازش نمونه‌ای با double (کند در M7) */
#define DSP_USE_Q15                    1   /* پردازش Fixed-Point 16bit */
#define DSP_USE_Q31                    1   /* پردازش Fixed-Point 32bit */

/* اگر float32 فعال است، نوع پیش‌فرض API ساده float32 خواهد بود.
 * اگر خاموش بود، اولین دقت فعال به ترتیب زیر انتخاب می‌شود: f64 -> q31 -> q15 */
#if DSP_USE_FLOAT32
  #define DSP_DEFAULT_PRECISION    DSP_PRECISION_ID_F32
#elif DSP_USE_FLOAT64
  #define DSP_DEFAULT_PRECISION    DSP_PRECISION_ID_F64
#elif DSP_USE_Q31
  #define DSP_DEFAULT_PRECISION    DSP_PRECISION_ID_Q31
#else
  #define DSP_DEFAULT_PRECISION    DSP_PRECISION_ID_Q15
#endif

/* ===========================================================================
 * 3) BACKEND SELECTION
 * ===========================================================================
 * 1 = از CMSIS-DSP (arm_math.h) به عنوان Backend استفاده شود.
 * 0 = از Backend داخلی (Cortex-M7 Optimized C) استفاده شود.
 *
 * نکته‌ی مهم معماری: انتخاب Backend در زمان کامپایل انجام می‌شود، نه زمان
 * اجرا. دلیل: dispatch داینامیک (vtable/function pointer) در مسیر
 * Sample-by-Sample هزینه‌ی indirect call (~10-30 سیکل) و مانع inlining و
 * pipeline شدن حلقه‌ها می‌شود. برای جزئیات، مستندات Architecture را ببینید.
 */
#define DSP_USE_CMSIS_DSP              0

/* ===========================================================================
 * 4) Fixed-Point رفتارهای عددی
 * ===========================================================================
 * DSP_FIXED_ACC_SATURATE: اگر 1 باشد، در Fixed-Point پس از هر ضرب-جمع،
 * accumulator به‌صورت اشباع (saturating) جمع می‌شود (ایمن‌تر، کمی کندتر).
 * اگر 0 باشد، تنها در پایان بلاک اشباع می‌شود (سریع‌تر، همان رفتار CMSIS).
 */
#define DSP_FIXED_ACC_SATURATE         0

/* ===========================================================================
 * 5) تخصیص حافظه — پیش‌فرض: صفر تخصیص داینامیک
 * ===========================================================================
 * کتابخانه همیشه با بافرهای کاربر/استاتیک کار می‌کند. اگر این گزینه 1 شود،
 * برخی ساختارها می‌توانند حافظه‌ی داخلی خود را با malloc بگیرند (فقط برای
 * پلتفرم‌هایی که heap دارند؛ در STM32H7 معمولاً خاموش بماند).
 */
#define DSP_USE_DYNAMIC_ALLOCATION     0

/* ===========================================================================
 * 6) API — نام‌های کوتاه
 * ===========================================================================
 * با 1 شدن، ماکروهایی مانند FIR_Init معادل dsp_fir_init_* تعریف می‌شوند تا
 * استفاده‌ی ساده‌تر شود. در پروژه‌های بزرگ بهتر است 0 شود تا فضای نام آلوده
 * نشود.
 */
#define DSP_USE_SHORT_ALIASES          1

/* ===========================================================================
 * 7) محدودیت‌های طراحی (برای بافرهای داخلی Design-Time)
 * =========================================================================== */
#define DSP_MAX_DESIGN_ORDER           16  /* حداکثر مرتبه‌ی پروتوتایپ */
#define DSP_MAX_SOS_STAGES             16  /* حداکثر طبقات SOS (مرتبه‌ی 32 برای BP/BS) */
#define DSP_MAX_MEDIAN_WINDOW          31
#define DSP_MAX_SG_WINDOW              31
#define DSP_MAX_ADAPTIVE_LEN         64   /* حداکثر طول فیلتر تطبیقی (برای بافر داخلی RLS) */

#endif /* !DSP_CONFIG_FILE */
#endif /* DSP_CONFIG_H */
