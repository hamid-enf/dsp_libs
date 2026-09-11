/**
 * @file dsp_fir_q31.c — FIR Q31 (accumulator Q63 + اشباع)
 */
#include "dsp_port.h"
#include "dsp_fir.h"

#define DSP_FIR_T_ENABLED  (DSP_ENABLE_FIR && DSP_USE_Q31)
#define DSP_FIR_T_TYPE     dsp_q31_t
#define DSP_FIR_T_ACC      dsp_q63_t
#define DSP_FIR_T_SUFFIX   q31
#define DSP_FIR_T_FLOAT    0
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_T_CMSIS  1
  #define DSP_FIR_TPL_ARM_INIT(p, c, t, bs, s) \
      arm_fir_init_q31(&(p)->arm, (t), (const q31_t *)(const void *)(c), (q31_t *)(void *)(s), (bs))
  #define DSP_FIR_TPL_ARM_PROC(p, s, d, n) \
      arm_fir_q31(&(p)->arm, (const q31_t *)(const void *)(s), (q31_t *)(void *)(d), (n))
#else
  #define DSP_FIR_T_CMSIS  0
#endif
#define DSP_FIR_TPL_OUT(acc, p) \
    DSP_Q31_SAT(((acc) + ((dsp_q63_t)1 << ((p)->shift - 1))) >> (p)->shift)
#define DSP_FIR_TPL_SATADD(a, b)     dsp_q63_sat_add((a), (b))
#if DSP_FIXED_ACC_SATURATE
  #define DSP_FIR_TPL_ACC(acc, expr) (acc) = dsp_q63_sat_add((acc), (expr))
#else
  #define DSP_FIR_TPL_ACC(acc, expr) (acc) += (expr)
#endif

#include "dsp_fir_tpl.h"
