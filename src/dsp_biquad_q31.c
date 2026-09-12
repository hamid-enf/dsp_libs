/**
 * @file dsp_biquad_q31.c — Biquad Q31 (accumulator Q63)
 */
#include "dsp_port.h"
#include "dsp_biquad.h"

#define DSP_BQ_T_ENABLED  (DSP_ENABLE_BIQUAD && DSP_USE_Q31)
#define DSP_BQ_T_TYPE     dsp_q31_t
#define DSP_BQ_T_ACC      dsp_q63_t
#define DSP_BQ_T_SUFFIX   q31
#define DSP_BQ_T_FLOAT    0
#define DSP_BQ_T_FC       dsp_f32_t
#define DSP_BQ_TPL_OUT(acc, p)   dsp_shift_round_q31((acc), (int32_t)(p)->shift)
#define DSP_BQ_TPL_SATADD(a, b)  dsp_q63_sat_add((a), (b))
#if DSP_FIXED_ACC_SATURATE
  #define DSP_BQ_TPL_ACC(acc, expr) (acc) = dsp_q63_sat_add((acc), (expr))
#else
  #define DSP_BQ_TPL_ACC(acc, expr) (acc) += (expr)
#endif
#define DSP_BQ_TPL_PARAM_STORE(p, c) \
    do { dsp_q31_t qc[5]; uint8_t sh; \
         dsp_biquad_quant_q31((c), qc, &sh); \
         (p)->b0=qc[0]; (p)->b1=qc[1]; (p)->b2=qc[2]; (p)->a1=qc[3]; (p)->a2=qc[4]; \
         (p)->shift=sh; } while (0)

#include "dsp_biquad_tpl.h"
#if DSP_USE_Q31

dsp_err_t dsp_biquad_set_coeffs_q31(dsp_biquad_q31_t *p, dsp_q31_t b0, dsp_q31_t b1,
                                    dsp_q31_t b2, dsp_q31_t a1, dsp_q31_t a2, uint8_t shift)
{
    if (p == NULL) return DSP_ERR_NULL_PTR;
    if (shift < 1 || shift > 34) return DSP_ERR_INVALID_PARAMETER;
    p->b0 = b0; p->b1 = b1; p->b2 = b2; p->a1 = a1; p->a2 = a2;
    p->shift = shift;
    p->type = DSP_BIQUAD_RAW;
    dsp_biquad_reset_q31(p);
    return DSP_OK;
}

dsp_err_t dsp_biquad_get_coeffs_q31(const dsp_biquad_q31_t *p, dsp_q31_t c[5], uint8_t *shift)
{
    if (p == NULL || c == NULL || shift == NULL) return DSP_ERR_NULL_PTR;
    c[0]=p->b0; c[1]=p->b1; c[2]=p->b2; c[3]=p->a1; c[4]=p->a2;
    *shift = p->shift;
    return DSP_OK;
}
#endif /* DSP_USE_Q31 */
