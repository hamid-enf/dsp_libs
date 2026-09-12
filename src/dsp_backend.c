/**
 * @file dsp_backend.c — اطلاعات Backend فعال
 */
#include "dsp_backend.h"

void dsp_backend_get_info(dsp_backend_info_t *info)
{
    if (info == NULL) return;
    info->id = dsp_backend_get_active();
#if DSP_USE_CMSIS_DSP
    info->supports_fir_direct    = DSP_TRUE;
    info->supports_fir_transposed = DSP_FALSE;   /* CMSIS فقط Direct دارد */
    info->supports_fir_symmetric  = DSP_FALSE;
    info->supports_sos_tdf2       = DSP_TRUE;
    info->supports_f32            = DSP_TRUE;
    info->supports_f64            = DSP_FALSE;
    info->supports_q15            = DSP_TRUE;
    info->supports_q31            = DSP_TRUE;
#else
    info->supports_fir_direct    = DSP_TRUE;
    info->supports_fir_transposed = DSP_TRUE;
    info->supports_fir_symmetric  = DSP_TRUE;
    info->supports_sos_tdf2       = DSP_TRUE;
    info->supports_f32            = (DSP_USE_FLOAT32 != 0);
    info->supports_f64            = (DSP_USE_FLOAT64 != 0);
    info->supports_q15            = (DSP_USE_Q15 != 0);
    info->supports_q31            = (DSP_USE_Q31 != 0);
#endif
}
