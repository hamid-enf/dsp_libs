# IIR — پیاده‌سازی

- **SOS Cascade (توصیه‌شده):** `dsp_iir_init(..., sos=1)` — ضرایب
  `[b0 b1 b2 a1 a2] × stages`، فرم TDF2 یا DF1، state = 2×stages (TDF2)
  یا 4×stages (DF1).
- **مستقیم:** `sos=0` — ضرایب `[b0..bN | a1..aN]`، فرم DF1/DF2/TDF2.
- **Fixed-Point:** فقط DF1 و SOS/DF1 (به نقد معماری مراجعه کنید).
- **به‌روزرسانی:** `dsp_iir_update_coeffs*` بین بلاک‌ها.

```c
/* طراحی + اجرا (Butterworth 4th order, 100Hz @ 1kHz) */
double sos64[2*5]; uint16_t nsec;
dsp_design_params_t cfg = { .fs=1000, .fc=100, .type=DSP_FILTER_LP,
                            .order=4, .pSOS=sos64, .pNumSections=&nsec };
dsp_design_butterworth(&cfg);
dsp_f32_t sos32[2*5]; dsp_design_sos_f64_to_f32(sos64, nsec, sos32);
dsp_f32_t st[2*2];
dsp_iir_f32_t iir;
dsp_iir_init_f32(&iir, DSP_TDF2, 4, sos32, st, 4, 1);
```
