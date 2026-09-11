# نقشه‌ی راه توسعه

## نسخه 1.x (پایه — تحویل‌شده)
- هسته‌ی کامل FIR/IIR/Biquad با ۴ دقت، دو Backend
- موتور طراحی Butterworth/Chebyshev I&II/Elliptic/Bessel (اعتبارسنجی scipy)
- فیلترهای کاربردی، تطبیقی، Kalman 1D، Resampling، Utils
- تست‌ها، بنچمارک، ۸ مثال، مستندات فارسی

## نسخه 1.1
- **Kalman چندبعدی + EKF** (ماتریس‌های P/K/H — معماری از قبل آماده است)
- بهینه‌سازی SIMD برای Q15/Q31 (SMLAD/SMLALD، و در MVE برای M85)
- فیلتر CIC (برای Decim پرسرعت)
- پشتیبانی IAR/Keil برای ماکروهای Memory Placement
- Histogram-Median برای پنجره‌های بزرگ

## نسخه 1.2
- STFT/Goertzel برای تحلیل طیفی داخلی (کاهش وابستگی به CMSIS)
- طراح FIR بهینه (Parks-McClellan / Remez)
- Rational SRC با فیلتر چندمرحله‌ای
- پروفایلینگ خودکار با DWT

## سازگاری API
تا نسخه‌ی 2.0: افزودن API فقط با MINOR/PATCH؛ تغییرات مخرب با MAJOR.
`DSP_VERSION_CHECK` برای مدیریت سازگاری.
