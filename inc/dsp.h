/**
 ******************************************************************************
 * @file    dsp.h
 * @brief   هدر اصلی کتابخانه DSP برای STM32H7 (Cortex-M7)
 *
 * فقط ماژول‌هایی که در dsp_config.h فعال شده‌اند، در دسترس قرار می‌گیرند.
 *
 * معماری:
 *   Application
 *      |   Public DSP API (این فایل + ماژول‌ها)
 *   Filter Abstraction
 *      |   Backend Interface (dsp_backend.h)
 *   +---------------------+
 *   |                     |
 * Custom DSP Backend   CMSIS-DSP Backend
 *   |                     |
 * Cortex-M7 Optimized  ARM Optimized
 ******************************************************************************
 */

#ifndef DSP_H
#define DSP_H

#include "dsp_config.h"
#include "dsp_types.h"
#include "dsp_port.h"

/* --- ماژول‌های هسته --- */
#if DSP_ENABLE_FIR
  #include "dsp_fir.h"
#endif
#if DSP_ENABLE_IIR
  #include "dsp_iir.h"
#endif
#if DSP_ENABLE_BIQUAD
  #include "dsp_biquad.h"
#endif

/* --- طراحی فیلتر --- */
#if DSP_ENABLE_BUTTERWORTH || DSP_ENABLE_CHEBYSHEV || DSP_ENABLE_ELLIPTIC || DSP_ENABLE_BESSEL
  #include "dsp_design.h"
#endif

/* --- فیلترهای کاربردی --- */
#if DSP_ENABLE_MOVING_AVERAGE
  #include "dsp_moving.h"
#endif
#if DSP_ENABLE_MEDIAN
  #include "dsp_median.h"
#endif
#if DSP_ENABLE_SAVGOL
  #include "dsp_savgol.h"
#endif
#if DSP_ENABLE_DCBLOCK
  #include "dsp_dcblock.h"
#endif
#if DSP_ENABLE_NOTCH
  #include "dsp_notch.h"
#endif

/* --- تطبیقی و تخمین --- */
#if DSP_ENABLE_LMS || DSP_ENABLE_NLMS || DSP_ENABLE_RLS
  #include "dsp_adaptive.h"
#endif
#if DSP_ENABLE_KALMAN
  #include "dsp_kalman.h"
#endif

/* --- تغییر نرخ نمونه‌برداری --- */
#if DSP_ENABLE_RESAMPLING
  #include "dsp_resample.h"
#endif

/* --- بلوک‌های عمومی --- */
#if DSP_ENABLE_SIGNAL_UTILS
  #include "dsp_utils.h"
#endif

/* --- Backend --- */
#include "dsp_backend.h"

#endif /* DSP_H */
