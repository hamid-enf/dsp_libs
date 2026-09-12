/**
 * @file dsp_fir_f64.c — FIR float64 (فقط Backend داخلی؛ CMSIS پشتیبانی ندارد)
 */
#include "dsp_port.h"
#include "dsp_fir.h"

#define DSP_FIR_T_ENABLED  (DSP_ENABLE_FIR && DSP_USE_FLOAT64)
#define DSP_FIR_T_TYPE     dsp_f64_t
#define DSP_FIR_T_ACC      dsp_f64_t
#define DSP_FIR_T_SUFFIX   f64
#define DSP_FIR_T_FLOAT    1
#define DSP_FIR_T_CMSIS    0
#define DSP_FIR_TPL_OUT(acc, p)      ((dsp_f64_t)(acc))
#define DSP_FIR_TPL_SATADD(a, b)     ((a) + (b))
#define DSP_FIR_TPL_ACC(acc, expr)   (acc) += (expr)

#include "dsp_fir_tpl.h"
