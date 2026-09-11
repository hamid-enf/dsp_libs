/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "printf_redirect.h"
#include "dsp.h"

/* مثال‌های header-only کتابخانه — هرکدام init و process مستقل دارند. */
#include "../../dsp_libs/examples/filters/01_fir_example.h"
#include "../../dsp_libs/examples/filters/02_iir_example.h"
#include "../../dsp_libs/examples/filters/03_biquad_example.h"
#include "../../dsp_libs/examples/filters/04_butterworth_example.h"
#include "../../dsp_libs/examples/filters/05_chebyshev_example.h"
#include "../../dsp_libs/examples/filters/06_elliptic_example.h"
#include "../../dsp_libs/examples/filters/07_bessel_example.h"
#include "../../dsp_libs/examples/filters/08_moving_average_example.h"
#include "../../dsp_libs/examples/filters/09_median_example.h"
#include "../../dsp_libs/examples/filters/10_savgol_example.h"
#include "../../dsp_libs/examples/filters/11_dc_blocker_example.h"
#include "../../dsp_libs/examples/filters/12_notch_example.h"
#include "../../dsp_libs/examples/filters/13_adaptive_example.h"
#include "../../dsp_libs/examples/filters/14_kalman_example.h"
#include "../../dsp_libs/examples/filters/15_resampling_example.h"
#include "../../dsp_libs/examples/filters/16_signal_utils_example.h"
#include "../../dsp_libs/examples/realtime_audio_callback.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* -------------------------------------------------------------------------- */
/* اندازه‌گیری زمان اجرای مثال‌ها                                             */
/* -------------------------------------------------------------------------- */
/*
 * این اندازه‌گیری با DWT انجام می‌شود و به HAL/تایمر Cube وابسته نیست.
 * DWT یک‌بار در main، بعد از SystemClock_Config و قبل از مثال‌ها فعال می‌شود.
 * خروجی شامل سیکل به‌ازای هر call و زمان تقریبی برحسب میکروثانیه است.
 *
 * نکته: برای معتبر بودن تبدیل سیکل به زمان، مقدار SystemCoreClock باید با
 * کلاک واقعی CPU برابر باشد؛ HAL بعد از SystemClock_Config آن را به‌روز می‌کند.
 */
typedef dsp_err_t (*dsp_demo_fn_t)(void);

static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0u;
}

static uint32_t DWT_Now(void)
{
    return DWT->CYCCNT;
}

static dsp_err_t Demo_Measure(const char *label,
                              dsp_demo_fn_t fn,
                              uint32_t repeats,
                              uint32_t calls_per_repeat,
                              uint8_t warmup)
{
    uint32_t i;
    uint32_t start, end;
    uint64_t total_cycles = 0u;
    dsp_err_t status = DSP_OK;
    uint64_t denominator;
    uint64_t avg_cycles;
    uint64_t us_x1000;
    uint32_t us_whole, us_fraction;

    if (label == NULL || fn == NULL || repeats == 0u || calls_per_repeat == 0u) {
        return DSP_ERR_INVALID_PARAMETER;
    }

    if (warmup) {
        status = fn();
        if (status != DSP_OK) {
            printf("[ERROR] %-32s -> %s\r\n", label, dsp_err_str(status));
            return status;
        }
    }

    start = DWT_Now();
    for (i = 0u; i < repeats; ++i) {
        status = fn();
        if (status != DSP_OK) break;
    }
    end = DWT_Now();

    /* unsigned subtraction عمداً wrap-around شمارنده‌ی ۳۲بیتی را پوشش می‌دهد. */
    total_cycles = (uint64_t)(uint32_t)(end - start);
    denominator = (uint64_t)((i == 0u) ? 1u : i) * calls_per_repeat;
    avg_cycles = total_cycles / denominator;

    if (SystemCoreClock != 0u) {
        us_x1000 = (total_cycles * 1000000ull * 1000ull) /
                   ((uint64_t)((i == 0u) ? 1u : i) * SystemCoreClock * calls_per_repeat);
    } else {
        us_x1000 = 0u;
    }
    us_whole = (uint32_t)(us_x1000 / 1000ull);
    us_fraction = (uint32_t)(us_x1000 % 1000ull);

    printf("[TIME] %-32s : %lu cycles/call | %lu.%03lu us/call | %s\r\n",
           label,
           (unsigned long)avg_cycles,
           (unsigned long)us_whole,
           (unsigned long)us_fraction,
           dsp_err_str(status));
    return status;
}

/* -------------------------------------------------------------------------- */
/* ورودی مشترک تست                                                             */
/* -------------------------------------------------------------------------- */
#define DSP_DEMO_BLOCK  128u
#define DSP_DEMO_FRAMES 128u
#define DSP_DEMO_REPEAT 10u

static dsp_f32_t demo_input[DSP_DEMO_BLOCK];
static dsp_f32_t demo_output[DSP_DEMO_BLOCK];
static dsp_f32_t demo_resample_output[DSP_DEMO_BLOCK * 2u];
static dsp_q15_t demo_q15_input[DSP_DEMO_BLOCK];
static dsp_q15_t demo_q15_output[DSP_DEMO_BLOCK];
static int16_t demo_pcm[2u * DSP_DEMO_FRAMES];

static void Demo_FillInput(void)
{
    uint32_t i;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        dsp_f32_t t = (dsp_f32_t)i / 48000.0f;
        demo_input[i] = 0.55f * dsp_sin_f32(DSP_TWO_PI_F * 440.0f * t)
                      + 0.18f * dsp_sin_f32(DSP_TWO_PI_F * 4000.0f * t)
                      + 0.03f;
        demo_q15_input[i] = (dsp_q15_t)(demo_input[i] * 32767.0f);
        demo_pcm[2u * i] = demo_q15_input[i];
        demo_pcm[2u * i + 1u] = demo_q15_input[i];
    }
}

/* -------------------------------------------------------------------------- */
/* Initهای مثال‌ها                                                             */
/* -------------------------------------------------------------------------- */
static dsp_err_t Demo_Init_FIR(void)          { return dsp_example_fir_init(); }
static dsp_err_t Demo_Init_IIR(void)          { return dsp_example_iir_init(); }
static dsp_err_t Demo_Init_Biquad(void)       { return dsp_example_biquad_eq_init(48000u); }
static dsp_err_t Demo_Init_Butterworth(void)  { return dsp_example_butterworth_init(48000u, 4000.0); }
static dsp_err_t Demo_Init_Chebyshev(void)    { return dsp_example_chebyshev_init(48000u, 4000.0); }
static dsp_err_t Demo_Init_Elliptic(void)     { return dsp_example_elliptic_init(48000u, 4000.0); }
static dsp_err_t Demo_Init_Bessel(void)       { return dsp_example_bessel_init(48000u, 4000.0); }
static dsp_err_t Demo_Init_Moving(void)       { return dsp_example_moving_init(); }
static dsp_err_t Demo_Init_Median(void)       { return dsp_example_median_init(); }
static dsp_err_t Demo_Init_SavitzkyGolay(void){ return dsp_example_savgol_init(); }
static dsp_err_t Demo_Init_DCBlocker(void)    { return dsp_example_dcblock_init(); }
static dsp_err_t Demo_Init_Notch(void)        { return dsp_example_notch_init(48000u, 50.0f); }
static dsp_err_t Demo_Init_Adaptive(void)     { return dsp_example_adaptive_init(); }
static dsp_err_t Demo_Init_Kalman(void)       { return dsp_example_kalman_init(0.0f); }
static dsp_err_t Demo_Init_Resampling(void)   { return dsp_example_resampling_init(); }
static dsp_err_t Demo_Init_Utils(void)        { return dsp_example_utils_init(); }
static dsp_err_t Demo_Init_AudioChain(void)   { return dsp_example_audio_init(48000u, 2u); }

static dsp_err_t Demo_InitializeAll(void)
{
    static const struct {
        const char *name;
        dsp_demo_fn_t fn;
    } init_list[] = {
        {"FIR init",           Demo_Init_FIR},
        {"IIR init",           Demo_Init_IIR},
        {"Biquad EQ init",     Demo_Init_Biquad},
        {"Butterworth init",   Demo_Init_Butterworth},
        {"Chebyshev init",     Demo_Init_Chebyshev},
        {"Elliptic init",      Demo_Init_Elliptic},
        {"Bessel init",        Demo_Init_Bessel},
        {"Moving Average init",Demo_Init_Moving},
        {"Median init",        Demo_Init_Median},
        {"Savitzky-Golay init",Demo_Init_SavitzkyGolay},
        {"DC Blocker init",    Demo_Init_DCBlocker},
        {"Notch init",         Demo_Init_Notch},
        {"Adaptive init",      Demo_Init_Adaptive},
        {"Kalman init",        Demo_Init_Kalman},
        {"Resampling init",    Demo_Init_Resampling},
        {"Signal Utils init",  Demo_Init_Utils},
        {"Audio chain init",   Demo_Init_AudioChain}
    };
    uint32_t i;
    dsp_err_t e;

    printf("\r\n--- FILTER INITIALIZATION TIME ---\r\n");
    for (i = 0u; i < (uint32_t)(sizeof init_list / sizeof init_list[0]); ++i) {
        e = Demo_Measure(init_list[i].name, init_list[i].fn, 1u, 1u, 0u);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

/* -------------------------------------------------------------------------- */
/* Process wrapperها: هر wrapper یک یا چند call واقعی از API را اجرا می‌کند.   */
/* -------------------------------------------------------------------------- */
static dsp_err_t Demo_Process_FIR_Block(void)
{
    return dsp_example_fir_process_block(demo_input, demo_output, DSP_DEMO_BLOCK);
}
static dsp_err_t Demo_Process_FIR_Sample(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_fir_process_sample(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_IIR_Block(void)
{
    return dsp_example_iir_process_block(demo_input, demo_output, DSP_DEMO_BLOCK);
}
static dsp_err_t Demo_Process_IIR_Sample(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_iir_process_sample(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Biquad(void)
{
    return dsp_example_biquad_eq_process_pcm16(demo_pcm, DSP_DEMO_FRAMES);
}
static dsp_err_t Demo_Process_Butterworth(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_butterworth_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Chebyshev(void)
{
    uint32_t i;
    dsp_f32_t type1, type2;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_chebyshev_process(demo_input[i], &type1, &type2);
        demo_output[i] = type1;
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Elliptic(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_elliptic_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Bessel(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_bessel_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_SMA(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_sma_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_WMA(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_wma_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_EMA(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_ema_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Median(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_median_process_f32(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_SavitzkyGolay(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_savgol_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_DCBlocker(void)
{
    return dsp_example_dcblock_process_block(demo_input, demo_output, DSP_DEMO_BLOCK);
}
static dsp_err_t Demo_Process_Notch(void)
{
    return dsp_example_notch_process(demo_input[0], &demo_output[0]);
}
static dsp_err_t Demo_Process_LMS(void)
{
    uint32_t i;
    dsp_f32_t y, eout;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_lms_process(demo_input[i], demo_input[i] * 0.8f, &y, &eout);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_NLMS(void)
{
    uint32_t i;
    dsp_f32_t y, eout;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_nlms_process(demo_input[i], demo_input[i] * 0.8f, &y, &eout);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Kalman(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_kalman_process(demo_input[i], &demo_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Decimator(void)
{
    return dsp_example_decim_process(demo_input, demo_output, DSP_DEMO_BLOCK / 2u);
}
static dsp_err_t Demo_Process_Interpolator(void)
{
    return dsp_example_interp_process(demo_input, demo_resample_output, DSP_DEMO_BLOCK);
}
static dsp_err_t Demo_Process_Median_Q15(void)
{
    uint32_t i;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_median_process_q15(demo_q15_input[i], &demo_q15_output[i]);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Utils_Sample(void)
{
    uint32_t i;
    dsp_f32_t limited, envelope;
    dsp_err_t e = DSP_OK;
    for (i = 0u; i < DSP_DEMO_BLOCK; ++i) {
        e = dsp_example_utils_process_sample(demo_input[i], &limited, &envelope);
        if (e != DSP_OK) break;
    }
    return e;
}
static dsp_err_t Demo_Process_Utils_RMS(void)
{
    (void)dsp_example_utils_rms(demo_input, DSP_DEMO_BLOCK);
    return DSP_OK;
}
static dsp_err_t Demo_Process_Utils_Gain(void)
{
    dsp_example_utils_gain(demo_input, demo_output, DSP_DEMO_BLOCK, 0.75f);
    return DSP_OK;
}
static dsp_err_t Demo_Process_AudioChain(void)
{
    return dsp_example_audio_process_pcm16(demo_pcm, 2u * DSP_DEMO_FRAMES, 2u);
}

static dsp_err_t Demo_RunProcessMeasurements(void)
{
    static const struct {
        const char *name;
        dsp_demo_fn_t fn;
        uint32_t calls;
    } process_list[] = {
        {"FIR process block",       Demo_Process_FIR_Block,       1u},
        {"FIR process sample",      Demo_Process_FIR_Sample,      DSP_DEMO_BLOCK},
        {"IIR process block",       Demo_Process_IIR_Block,       1u},
        {"IIR process sample",      Demo_Process_IIR_Sample,      DSP_DEMO_BLOCK},
        {"Biquad EQ PCM16",         Demo_Process_Biquad,           DSP_DEMO_FRAMES},
        {"Butterworth process",     Demo_Process_Butterworth,      DSP_DEMO_BLOCK},
        {"Chebyshev I + II",        Demo_Process_Chebyshev,        DSP_DEMO_BLOCK},
        {"Elliptic process",        Demo_Process_Elliptic,         DSP_DEMO_BLOCK},
        {"Bessel process",          Demo_Process_Bessel,           DSP_DEMO_BLOCK},
        {"SMA process",             Demo_Process_SMA,              DSP_DEMO_BLOCK},
        {"WMA process",             Demo_Process_WMA,              DSP_DEMO_BLOCK},
        {"EMA process",             Demo_Process_EMA,              DSP_DEMO_BLOCK},
        {"Median f32 process",      Demo_Process_Median,            DSP_DEMO_BLOCK},
        {"Median Q15 process",      Demo_Process_Median_Q15,        DSP_DEMO_BLOCK},
        {"Savitzky-Golay process",  Demo_Process_SavitzkyGolay,     DSP_DEMO_BLOCK},
        {"DC Blocker block",        Demo_Process_DCBlocker,         1u},
        {"Notch process",           Demo_Process_Notch,             1u},
        {"LMS process",             Demo_Process_LMS,                DSP_DEMO_BLOCK},
        {"NLMS process",            Demo_Process_NLMS,               DSP_DEMO_BLOCK},
        {"Kalman process",          Demo_Process_Kalman,             DSP_DEMO_BLOCK},
        {"Decimator process",       Demo_Process_Decimator,           DSP_DEMO_BLOCK / 2u},
        {"Interpolator process",    Demo_Process_Interpolator,        DSP_DEMO_BLOCK},
        {"Utils sample",            Demo_Process_Utils_Sample,         DSP_DEMO_BLOCK},
        {"Utils RMS",               Demo_Process_Utils_RMS,             1u},
        {"Utils gain",              Demo_Process_Utils_Gain,             1u},
        {"Realtime audio chain",    Demo_Process_AudioChain,       2u * DSP_DEMO_FRAMES}
    };
    uint32_t i;
    dsp_err_t e;

    printf("\r\n--- FILTER PROCESSING TIME ---\r\n");
    for (i = 0u; i < (uint32_t)(sizeof process_list / sizeof process_list[0]); ++i) {
        e = Demo_Measure(process_list[i].name, process_list[i].fn,
                         DSP_DEMO_REPEAT, process_list[i].calls, 1u);
        if (e != DSP_OK) return e;
    }
    return DSP_OK;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  DWT_Init();
  Demo_FillInput();

  printf("\r\nDSP examples on STM32H7\r\n");
  printf("SystemCoreClock = %lu Hz\r\n", (unsigned long)SystemCoreClock);

  if (Demo_InitializeAll() != DSP_OK) {
    Error_Handler();
  }
  if (Demo_RunProcessMeasurements() != DSP_OK) {
    Error_Handler();
  }

  uint32_t last = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(HAL_GetTick() > last + 1000 ){

		  HAL_GPIO_TogglePin(led_GPIO_Port, led_Pin);

		  last = HAL_GetTick();
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
