/* ابزار تست مشترک */
#ifndef DSP_TEST_UTIL_H
#define DSP_TEST_UTIL_H

#include <stdio.h>
#include <math.h>
#include <string.h>

extern int dsp_test_fails;

#define TCHECK(cond, msg) \
    do { if (!(cond)) { printf("  FAIL: %s (line %d)\n", msg, __LINE__); dsp_test_fails++; } } while (0)

#define TCHECK_NEAR(a, b, tol, msg) \
    do { double _d = fabs((double)(a) - (double)(b)); \
         if (_d > (tol)) { printf("  FAIL: %s : %g vs %g (diff %g)\n", msg, (double)(a), (double)(b), _d); dsp_test_fails++; } } while (0)

#define TEST_SECTION(name) printf("\n== %s ==\n", name)

#endif
