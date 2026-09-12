# Moving Average — SMA / WMA / EMA

## ۱. معرفی
ساده‌ترین فیلترهای هموارسازی؛ هر سه O(1) یا O(W) با پیاده‌سازی مستقیم.

## ۲. کاربرد
نرم‌سازی خوانش حسگر، پیش‌پردازش قبل از کنترلر/FFT.

## ۳. مزایا
- بسیار سبک و سریع (SMA: O(1) با Running Sum)
- بدون تنظیم پارامتر پیچیده

## ۴. معایب
- گزینش‌پذیری ضعیف (لوب‌های جانبی)
- تأخیر (W−1)/2 (SMA/WMA)

## ۵. محدودیت‌ها
- برای حذف نویز پهن‌باند به W بزرگ نیاز دارد → تأخیر زیاد

## ۶. مدل ریاضی

**SMA:**
$$y[n] = \frac{1}{W}\sum_{k=0}^{W-1} x[n-k] \quad\Longleftrightarrow\quad
y[n] = y[n-1] + \frac{x[n] - x[n-W]}{W}$$

**WMA:**
$$y[n] = \frac{\sum_{k=0}^{W-1} w_k\, x[n-k]}{\sum_k w_k}$$

**EMA:**
$$y[n] = \alpha\, x[n] + (1-\alpha)\, y[n-1]$$

## ۷–۸. متغیرها
$W$: پنجره، $w_k$: وزن‌ها، $\alpha$: ضریب هموارسازی (0..1)؛
$\alpha$ کوچک → هموارتر/کندتر.

## ۹. انتخاب پارامترها
- SMA: W ≈ دوره‌ی نویز. EMA: معادل زمان ثابت $\tau = \dfrac{1-\alpha}{\alpha}$
  نمونه؛ برای نویز ۵۰Hz با fs=1kHz → α ≈ 0.09.

## ۱۰–۱۴. اثرها
- افزایش W/کاهش α → نویز کمتر، تأخیر و «لَختی» بیشتر.
- SMA: پاسخ فرکانسی |sin(πfW/fs)/(W·sin(πf/fs))| — نال در f = fs/W.
- EMA: یک‌قطبی — −6dB/oct بعد از fc ≈ α·fs/(2π).
- Phase: SMA/WMA خطی؛ EMA غیرخطی.
- Stability: همیشه پایدار.

## ۱۵–۱۶. CPU/RAM
SMA/EMA: O(1)، RAM = W (SMA) یا ۱ (EMA). WMA: O(W).

## ۱۷–۲۲. مثال‌ها
مثال ۵ (حسگر + SMA)؛ Sample: `dsp_sma_process_f32`؛ Block: `dsp_sma_process_block_f32`؛
ADC: SMA قبل از نمایش؛ Audio: EMA برای Envelope/Level-meter.
