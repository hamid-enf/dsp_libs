/**
 ******************************************************************************
 * مثال ۸ — Sensor Fusion با Kalman 1D (ترکیب دو حسگر)
 *
 * سناریو: دو حسگر (مثلاً دو خروجی متفاوت از IMU و GPS برای یک کمیت).
 * هر خوانش با Kalman پردازش می‌شود؛ Q و R بر اساس اعتماد به هر حسگر.
 *
 * نکته: این مثال Kalman 1D است؛ برای فیوژن واقعی چندحسگری چندبعدی،
 * ساختار را به حالت برداری گسترش دهید (معماری به این منظور طراحی شده).
 ******************************************************************************
 */
#include "stm32h7xx_hal.h"
#include "dsp.h"

typedef struct {
    dsp_kalman1d_f32_t kf_a;   /* حسگر A (مثلاً IMU — نویز کم، Drift دارد) */
    dsp_kalman1d_f32_t kf_b;   /* حسگر B (مثلاً GPS — نویز زیاد، بدون Drift) */
} dsp_fusion1d_t;

static dsp_fusion1d_t fusion;

void fusion_init(dsp_fusion1d_t *f, dsp_f32_t x0)
{
    dsp_kalman1d_init_f32(&f->kf_a, x0, 1.0f, 1e-6f, 0.01f);   /* Q کوچک: به مدل اعتماد کن */
    dsp_kalman1d_init_f32(&f->kf_b, x0, 1.0f, 1e-3f, 1.0f);    /* R بزرگ: به اندازه‌گیری کم‌اعتماد */
}

dsp_f32_t fusion_update(dsp_fusion1d_t *f, dsp_f32_t za, dsp_f32_t zb)
{
    dsp_f32_t xa, xb;
    dsp_kalman1d_process_f32(&f->kf_a, za, &xa);
    dsp_kalman1d_process_f32(&f->kf_b, zb, &xb);
    /* ترکیب وزنی بر اساس کوواریانس خطا (ساده‌شده) */
    return (xa / f->kf_a.p + xb / f->kf_b.p) / (1.0f / f->kf_a.p + 1.0f / f->kf_b.p);
}

/* توابع نمونه — در پروژه‌ی واقعی از حسگرها می‌خوانید */
dsp_f32_t read_imu_estimate(void) { return 0.0f; }
dsp_f32_t read_gps_estimate(void)  { return 0.0f; }

int main(void)
{
    HAL_Init();
    /* ... حسگرها init ... */
    fusion_init(&fusion, 0.0f);

    for (;;) {
        dsp_f32_t za = read_imu_estimate();   /* مثلاً از مکمل‌فیلتر */
        dsp_f32_t zb = read_gps_estimate();
        dsp_f32_t fused = fusion_update(&fusion, za, zb);
        /* fused = بهترین تخمین ترکیبی */
        HAL_Delay(50);
    }
}
