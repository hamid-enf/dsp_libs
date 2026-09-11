/**
 * @file dsp_iir_q15.c — IIR Q15 (فقط SOS/TDF2 برای مرتبه‌های بالا)
 */
#include "dsp_port.h"
#include "dsp_iir.h"

#define DSP_IIR_T_ENABLED  (DSP_ENABLE_IIR && DSP_USE_Q15)
#define DSP_IIR_T_TYPE     dsp_q15_t
#define DSP_IIR_T_ACC      dsp_q31_t
#define DSP_IIR_T_SUFFIX   q15
#define DSP_IIR_T_FLOAT    0
#define DSP_IIR_T_HAVE_SHIFT 1
#define DSP_IIR_TPL_OUT(acc, p)  dsp_shift_round_q15((acc), (int32_t)(p)->shift)
#if DSP_FIXED_ACC_SATURATE
  #define DSP_IIR_TPL_ACCUM(acc, e) (acc) = dsp_q31_sat_add((acc), (e))
#else
  #define DSP_IIR_TPL_ACCUM(acc, e) (acc) += (e)
#endif

#include "dsp_iir_tpl.h"
