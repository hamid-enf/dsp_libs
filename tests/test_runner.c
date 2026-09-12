#include "dsp.h"
/* اجراکننده‌ی همه‌ی تست‌ها */
#include <stdio.h>
#include "test_util.h"

int dsp_test_fails = 0;

void dsp_test_core_all(void);
void dsp_test_utils_all(void);
void dsp_test_adaptive_all(void);
void dsp_test_resample_all(void);

int main(void)
{
    printf("===== STM32H7 DSP Library — Test Suite =====\n");
    printf("Backend: %s | Version: %s\n", dsp_backend_name(), dsp_version_string());
    dsp_test_core_all();
    dsp_test_utils_all();
    dsp_test_adaptive_all();
    dsp_test_resample_all();
    printf("\n===== RESULT: %s =====\n", dsp_test_fails ? "FAILED" : "ALL TESTS PASSED");
    return dsp_test_fails ? 1 : 0;
}
