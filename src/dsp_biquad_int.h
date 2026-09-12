/**
 ******************************************************************************
 * @file    dsp_biquad_int.h  (internal)
 * @brief   موتور محاسبه‌ی ضرایب Biquad (فرمول‌های RBJ) + کمی‌سازی Fixed-Point
 *
 * فرمول‌های RBJ (Robert Bristow-Johnson) — مرجع:
 *   https://www.musicdsp.org/en/latest/Filters/197-rbj-audio-eq-cookbook.html
 *
 * این فایل فقط در داخل کتابخانه استفاده می‌شود؛ API عمومی آن dsp_biquad.h است.
 ******************************************************************************
 */

#ifndef DSP_BIQUAD_INT_H
#define DSP_BIQUAD_INT_H

#include "dsp_types.h"

/* محاسبه‌ی 5 ضریب نرمال‌شده (b0 b1 b2 a1 a2 با a0=1) از پارامترها — f64 */
void dsp_biquad_rbj_f64(uint8_t type, uint32_t fs,
                        double fc, double q, double gain_db, double c[5]);

/* کمی‌سازی برای Q15: خروجی shift = 15 + s  (ساختار را ببینید) */
void dsp_biquad_quant_q15(const double c[5], dsp_q15_t *qc, uint8_t *shift);
/* کمی‌سازی برای Q31: خروجی shift = 31 + s */
void dsp_biquad_quant_q31(const double c[5], dsp_q31_t *qc, uint8_t *shift);

#endif /* DSP_BIQUAD_INT_H */
