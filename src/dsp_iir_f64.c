/**
 * @file dsp_iir_f64.c — IIR float64
 */
#include "dsp_port.h"
#include "dsp_iir.h"

#define DSP_IIR_T_ENABLED  (DSP_ENABLE_IIR && DSP_USE_FLOAT64)
#define DSP_IIR_T_TYPE     dsp_f64_t
#define DSP_IIR_T_ACC      dsp_f64_t
#define DSP_IIR_T_SUFFIX   f64
#define DSP_IIR_T_FLOAT    1
#define DSP_IIR_T_HAVE_SHIFT 0
#define DSP_IIR_TPL_OUT(acc, p)    ((dsp_f64_t)(acc))
#define DSP_IIR_TPL_ACCUM(acc, e)  (acc) += (e)

#include "dsp_iir_tpl.h"
