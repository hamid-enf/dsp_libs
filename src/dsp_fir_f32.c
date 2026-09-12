/**
 * @file dsp_fir_f32.c — FIR float32 (Backend داخلی یا CMSIS-DSP)
 */
#include "dsp_port.h"
#include "dsp_fir.h"

#define DSP_FIR_T_ENABLED  (DSP_ENABLE_FIR && DSP_USE_FLOAT32)
#define DSP_FIR_T_TYPE     dsp_f32_t
#define DSP_FIR_T_ACC      dsp_f32_t
#define DSP_FIR_T_SUFFIX   f32
#define DSP_FIR_T_FLOAT    1
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_T_CMSIS  1
  #define DSP_FIR_TPL_ARM_INIT(p, c, t, bs, s) \
      arm_fir_init_f32(&(p)->arm, (t), (const float32_t *)(const void *)(c), (s), (bs))
  #define DSP_FIR_TPL_ARM_PROC(p, s, d, n) \
      arm_fir_f32(&(p)->arm, (const float32_t *)(const void *)(s), (float32_t *)(void *)(d), (n))
#else
  #define DSP_FIR_T_CMSIS  0
#endif
#define DSP_FIR_TPL_OUT(acc, p)      ((dsp_f32_t)(acc))
#define DSP_FIR_TPL_SATADD(a, b)     ((a) + (b))
#define DSP_FIR_TPL_ACC(acc, expr)   (acc) += (expr)

#include "dsp_fir_tpl.h"
