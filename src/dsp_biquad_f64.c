/**
 * @file dsp_biquad_f64.c — Biquad float64
 */
#include "dsp_port.h"
#include "dsp_biquad.h"

#define DSP_BQ_T_ENABLED  (DSP_ENABLE_BIQUAD && DSP_USE_FLOAT64)
#define DSP_BQ_T_TYPE     dsp_f64_t
#define DSP_BQ_T_ACC      dsp_f64_t
#define DSP_BQ_T_SUFFIX   f64
#define DSP_BQ_T_FLOAT    1
#define DSP_BQ_T_FC       dsp_f64_t
#define DSP_BQ_TPL_OUT(acc, p)       ((dsp_f64_t)(acc))
#define DSP_BQ_TPL_SATADD(a, b)      ((a) + (b))
#define DSP_BQ_TPL_ACC(acc, expr)    (acc) += (expr)
#define DSP_BQ_TPL_PARAM_STORE(p, c) \
    do { (p)->b0=(dsp_f64_t)(c)[0]; (p)->b1=(dsp_f64_t)(c)[1]; (p)->b2=(dsp_f64_t)(c)[2]; \
         (p)->a1=(dsp_f64_t)(c)[3]; (p)->a2=(dsp_f64_t)(c)[4]; } while (0)

#include "dsp_biquad_tpl.h"
#if DSP_USE_FLOAT64

dsp_err_t dsp_biquad_set_coeffs_f64(dsp_biquad_f64_t *p, dsp_f64_t b0, dsp_f64_t b1,
                                    dsp_f64_t b2, dsp_f64_t a1, dsp_f64_t a2)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    p->b0 = b0; p->b1 = b1; p->b2 = b2; p->a1 = a1; p->a2 = a2;
    p->type = DSP_BIQUAD_RAW;
    dsp_biquad_reset_f64(p);
    return DSP_OK;
}
#endif /* DSP_USE_FLOAT64 */
