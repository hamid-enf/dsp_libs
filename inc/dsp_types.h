/**
 ******************************************************************************
 * @file    dsp_types.h
 * @brief   انواع پایه: نسخه‌بندی، کدهای خطا، شمارنده‌ها
 ******************************************************************************
 */

#ifndef DSP_TYPES_H
#define DSP_TYPES_H

#include "dsp_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * نسخه‌بندی کتابخانه
 * ------------------------------------------------------------------------- */
#define DSP_VERSION_MAJOR   1u
#define DSP_VERSION_MINOR   0u
#define DSP_VERSION_PATCH   0u
#define DSP_VERSION_STRING  "1.0.0"

#define DSP_VERSION_CHECK(major, minor, patch) \
    ((DSP_VERSION_MAJOR > (major)) || \
     (DSP_VERSION_MAJOR == (major) && DSP_VERSION_MINOR > (minor)) || \
     (DSP_VERSION_MAJOR == (major) && DSP_VERSION_MINOR == (minor) && DSP_VERSION_PATCH >= (patch)))

/* ---------------------------------------------------------------------------
 * کدهای خطا — تمام توابع عمومی dsp_err_t برمی‌گردانند.
 * DSP_OK = 0 و خطاها مثبت‌اند.
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_OK                      = 0,   /* موفق */
    DSP_ERR_NULL_PTR            = 1,   /* اشاره‌گر NULL */
    DSP_ERR_INVALID_PARAMETER   = 2,   /* پارامتر نامعتبر (مقدار، اندازه، ...) */
    DSP_ERR_INVALID_STATE       = 3,   /* وضعیت داخلی نامعتبر (مثلاً Reset نشده) */
    DSP_ERR_BUFFER_TOO_SMALL    = 4,   /* بافر کاربر کوچک‌تر از نیاز است */
    DSP_ERR_UNSUPPORTED         = 5,   /* حالت/دقت/ویژگی پشتیبانی‌نشده */
    DSP_ERR_OVERFLOW            = 6,   /* سرریز Fixed-Point در طراحی/پردازش */
    DSP_ERR_NOT_INITIALIZED     = 7,   /* نمونه Initialize نشده */
    DSP_ERR_FEATURE_DISABLED    = 8,   /* ماژول در dsp_config.h خاموش است */
    DSP_ERR_NO_MEMORY           = 9,   /* تخصیص داینامیک ممکن نشد (فقط در حالت Optional) */
    DSP_ERR_DESIGN_CONVERGENCE  = 10   /* الگوریتم طراحی همگرا نشد (مثلاً مرتبه‌ی کم) */
} dsp_err_t;

/* ---------------------------------------------------------------------------
 * Precision ها
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_PREC_F32 = 0,
    DSP_PREC_Q15 = 1,
    DSP_PREC_Q31 = 2,
    DSP_PREC_F64 = 3
} dsp_prec_t;

/* ---------------------------------------------------------------------------
 * انواع فیلتر (برای موتور طراحی)
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_FILTER_LP = 0,   /* Low-Pass   */
    DSP_FILTER_HP = 1,   /* High-Pass  */
    DSP_FILTER_BP = 2,   /* Band-Pass  */
    DSP_FILTER_BS = 3    /* Band-Stop  */
} dsp_filter_type_t;

/* ---------------------------------------------------------------------------
 * فرم‌های ساختاری فیلترها
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_FORM_DIRECT = 0,     /* Direct Form */
    DSP_FORM_TRANSPOSED = 1, /* Transposed Direct Form */
    DSP_FORM_SYMMETRIC = 2   /* بهره‌برداری از تقارن ضرایب FIR */
} dsp_form_t;

typedef enum {
    DSP_DF1  = 0,   /* Direct Form I      */
    DSP_DF2  = 1,   /* Direct Form II     */
    DSP_TDF2 = 2    /* Transposed DF II   (پیشنهادی از نظر پایداری عددی) */
} dsp_iir_form_t;

/* ---------------------------------------------------------------------------
 * نوع بی‌کواد (برای تنظیم خودکار ضرایب با فرمول‌های RBJ)
 * ------------------------------------------------------------------------- */
typedef enum {
    DSP_BIQUAD_LPF = 0,
    DSP_BIQUAD_HPF = 1,
    DSP_BIQUAD_BPF = 2,      /* با پهنای باند یکسان، مرکز = fc */
    DSP_BIQUAD_BSF = 3,
    DSP_BIQUAD_NOTCH = 4,
    DSP_BIQUAD_PEAK = 5,     /* Peaking EQ */
    DSP_BIQUAD_LSHELF = 6,
    DSP_BIQUAD_HSHELF = 7,
    DSP_BIQUAD_RAW = 8       /* ضرایب مستقیم، بدون محاسبه‌ی RBJ */
} dsp_biquad_type_t;

/* ---------------------------------------------------------------------------
 * تبدیل dB <-> ضریب ولتاژ
 * ------------------------------------------------------------------------- */
static inline dsp_f32_t dsp_db_to_lin_f32(dsp_f32_t db) { return dsp_pow_f32(10.0f, db * 0.05f); }
static inline dsp_f32_t dsp_lin_to_db_f32(dsp_f32_t lin) { return 20.0f * dsp_log10_f32(lin); }

/* ---------------------------------------------------------------------------
 * توابع عمومی
 * ------------------------------------------------------------------------- */
static inline const char *dsp_version_string(void) { return DSP_VERSION_STRING; }

static inline const char *dsp_err_str(dsp_err_t e)
{
    switch (e) {
        case DSP_OK:                     return "DSP_OK";
        case DSP_ERR_NULL_PTR:           return "DSP_ERR_NULL_PTR";
        case DSP_ERR_INVALID_PARAMETER:  return "DSP_ERR_INVALID_PARAMETER";
        case DSP_ERR_INVALID_STATE:      return "DSP_ERR_INVALID_STATE";
        case DSP_ERR_BUFFER_TOO_SMALL:   return "DSP_ERR_BUFFER_TOO_SMALL";
        case DSP_ERR_UNSUPPORTED:        return "DSP_ERR_UNSUPPORTED";
        case DSP_ERR_OVERFLOW:           return "DSP_ERR_OVERFLOW";
        case DSP_ERR_NOT_INITIALIZED:    return "DSP_ERR_NOT_INITIALIZED";
        case DSP_ERR_FEATURE_DISABLED:   return "DSP_ERR_FEATURE_DISABLED";
        case DSP_ERR_NO_MEMORY:          return "DSP_ERR_NO_MEMORY";
        case DSP_ERR_DESIGN_CONVERGENCE: return "DSP_ERR_DESIGN_CONVERGENCE";
        default:                         return "DSP_ERR_UNKNOWN";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* DSP_TYPES_H */
