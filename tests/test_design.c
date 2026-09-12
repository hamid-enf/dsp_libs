/* اعتبارسنجی متقابل موتور طراحی با scipy — مقایسه‌ی پاسخ اندازه در فرکانس‌ها */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "dsp.h"
#include "ref/design_reference.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", msg, __LINE__); fails++; } } while (0)

int main(void)
{
    dsp_design_params_t cfg;
    double sos[DSP_MAX_SOS_STAGES * 5];
    uint16_t nsec = 0;
    double tol = 2e-9;

    memset(&cfg, 0, sizeof(cfg));
    cfg.pSOS = sos;
    cfg.pNumSections = &nsec;

/* تنظیم کامل پیکربندی برای هر کیس — بدون وابستگی به state بین کیس‌ها */
#define RESET(fs_, fc_, fc2_, type_, order_) \
    do { memset(&cfg, 0, sizeof(cfg)); cfg.fs=(fs_); cfg.fc=(fc_); cfg.fc2=(fc2_); \
         cfg.type=(type_); cfg.order=(order_); cfg.pSOS=sos; cfg.pNumSections=&nsec; } while (0)

#define CASE(designfn, reffreqs, refmag, refn, tol_) \
    do { dsp_err_t _e = designfn(&cfg); \
         if (_e != DSP_OK) { printf("FAIL: %s design err %d\n", #designfn, _e); fails++; break; } \
         for (int _i = 0; _i < (refn); _i++) { \
             double _w = 2.0 * M_PI * (reffreqs)[_i] / REF_FS; \
             double _m = dsp_design_sos_magnitude(sos, nsec, _w); \
             double _d = fabs(_m - (refmag)[_i]); \
             if (_d > (tol_)) { printf("FAIL: %s @%.0fHz got %.10f exp %.10f (%.2e)\n", #designfn, (reffreqs)[_i], _m, (refmag)[_i], _d); fails++; } \
         } } while (0)

#if DSP_ENABLE_BUTTERWORTH
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 2);
    CASE(dsp_design_butterworth, ref_butter_lp2_freqs, ref_butter_lp2_mag, ref_butter_lp2_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 4);
    CASE(dsp_design_butterworth, ref_butter_lp4_freqs, ref_butter_lp4_mag, ref_butter_lp4_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_HP, 3);
    CASE(dsp_design_butterworth, ref_butter_hp3_freqs, ref_butter_hp3_mag, ref_butter_hp3_n, tol);
    RESET(REF_FS, 500, 2000, DSP_FILTER_BP, 2);
    CASE(dsp_design_butterworth, ref_butter_bp2_freqs, ref_butter_bp2_mag, ref_butter_bp2_n, tol);
    RESET(REF_FS, 500, 2000, DSP_FILTER_BS, 2);
    CASE(dsp_design_butterworth, ref_butter_bs2_freqs, ref_butter_bs2_mag, ref_butter_bs2_n, tol);
#endif

#if DSP_ENABLE_CHEBYSHEV
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 3); cfg.ripple_db = 1.0;
    CASE(dsp_design_chebyshev1, ref_cheby1_lp3_freqs, ref_cheby1_lp3_mag, ref_cheby1_lp3_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 4); cfg.ripple_db = 0.5;
    CASE(dsp_design_chebyshev1, ref_cheby1_lp4_freqs, ref_cheby1_lp4_mag, ref_cheby1_lp4_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_HP, 2); cfg.ripple_db = 1.0;
    CASE(dsp_design_chebyshev1, ref_cheby1_hp2_freqs, ref_cheby1_hp2_mag, ref_cheby1_hp2_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 3); cfg.stop_db = 30.0;
    CASE(dsp_design_chebyshev2, ref_cheby2_lp3_freqs, ref_cheby2_lp3_mag, ref_cheby2_lp3_n, tol);
    RESET(REF_FS, 500, 2000, DSP_FILTER_BP, 2); cfg.stop_db = 40.0;
    CASE(dsp_design_chebyshev2, ref_cheby2_bp2_freqs, ref_cheby2_bp2_mag, ref_cheby2_bp2_n, tol);
#endif

#if DSP_ENABLE_ELLIPTIC
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 3); cfg.ripple_db = 1.0; cfg.stop_db = 40.0;
    CASE(dsp_design_elliptic, ref_ellip_lp3_freqs, ref_ellip_lp3_mag, ref_ellip_lp3_n, 5e-9);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 5); cfg.ripple_db = 0.5; cfg.stop_db = 60.0;
    CASE(dsp_design_elliptic, ref_ellip_lp5_freqs, ref_ellip_lp5_mag, ref_ellip_lp5_n, 5e-9);
    RESET(REF_FS, 500, 2000, DSP_FILTER_BS, 2); cfg.ripple_db = 1.0; cfg.stop_db = 40.0;
    CASE(dsp_design_elliptic, ref_ellip_bs2_freqs, ref_ellip_bs2_mag, ref_ellip_bs2_n, 5e-9);
#endif

#if DSP_ENABLE_BESSEL
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 3);
    CASE(dsp_design_bessel, ref_bessel_lp3_freqs, ref_bessel_lp3_mag, ref_bessel_lp3_n, tol);
    RESET(REF_FS, REF_FC, 0, DSP_FILTER_LP, 4);
    CASE(dsp_design_bessel, ref_bessel_lp4_freqs, ref_bessel_lp4_mag, ref_bessel_lp4_n, tol);
#endif

    printf(fails ? "\n*** %d DESIGN FAILURES ***\n" : "\n*** ALL DESIGN TESTS PASSED (vs scipy) ***\n", fails);
    return fails ? 1 : 0;
}
