/**
 * @file dsp_fir_q15.c — FIR Q15 (accumulator Q31 + اشباع)
 */
#include "dsp_port.h"
#include "dsp_fir.h"

#define DSP_FIR_T_ENABLED  (DSP_ENABLE_FIR && DSP_USE_Q15)
#define DSP_FIR_T_TYPE     dsp_q15_t
#define DSP_FIR_T_ACC      dsp_q31_t
#define DSP_FIR_T_SUFFIX   q15
#define DSP_FIR_T_FLOAT    0
#if DSP_USE_CMSIS_DSP
  #define DSP_FIR_T_CMSIS  1
  #define DSP_FIR_TPL_ARM_INIT(p, c, t, bs, s) \
      arm_fir_init_q15(&(p)->arm, (t), (const q15_t *)(const void *)(c), (q15_t *)(void *)(s), (bs))
  #define DSP_FIR_TPL_ARM_PROC(p, s, d, n) \
      arm_fir_q15(&(p)->arm, (const q15_t *)(const void *)(s), (q15_t *)(void *)(d), (n))
#else
  #define DSP_FIR_T_CMSIS  0
#endif
#define DSP_FIR_TPL_OUT(acc, p) \
    DSP_Q15_SAT(((acc) + ((dsp_q31_t)1 << ((p)->shift - 1))) >> (p)->shift)
#define DSP_FIR_TPL_SATADD(a, b)     dsp_q31_sat_add((a), (b))
#if DSP_FIXED_ACC_SATURATE
  #define DSP_FIR_TPL_ACC(acc, expr) (acc) = dsp_q31_sat_add((acc), (expr))
#else
  #define DSP_FIR_TPL_ACC(acc, expr) (acc) += (expr)
#endif

#include "dsp_fir_tpl.h"
