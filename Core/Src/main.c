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




static uint32_t cyccnt_last;
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   // برای H7 لازم است
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    cyccnt_last = 0;
}

/* اختلاف سیکل از آخرین فراخوانی، به میکروثانیه — نسبت به فرکانس واقعی */
uint32_t micros_abs(void)
{
    static uint32_t last = 0;
    static uint64_t high = 0;
    uint32_t now = DWT->CYCCNT;
    if (now < last) high += 1ull << 32;   /* wrap-safe */
    last = now;
    return (uint32_t)((high | now) / (SystemCoreClock / 1000000u));
}




uint64_t cyccnt64(void)
{
    static uint32_t last = 0;
    static uint64_t high = 0;
    uint32_t now = DWT->CYCCNT;
    if (now < last) high += 1ull << 32;   /* wrap */
    last = now;
    return high | now;
}

//uint32_t micros_abs(void)
//{
//    return (uint32_t)(cyccnt64() / (SystemCoreClock / 1000000u));
//}

uint32_t t0=0,t1=0,t2=0,t3=0,t4=0,t5=0;




/*============================================*/
/*============================================*/
/*============================================*/
/*============================================*/
/*============================================*/
/*============================================*/

//#define DTCM_SECTION __attribute__((section(".dtcmram"), aligned(32)))
#define N 4096u
#define FS 100000.0f
static float input[N];
float Max_Wave=0.0f;





void Init_inputs(){

	  for (uint32_t n = 0; n < N; ++n) {
//	      input[n] = 0.8f * sinf(2.0f * 3.14159265358979323846f * 1000.0f * (float)n / FS);
	      float wave1 = (1.0)      * 0.8f *sinf(2.0f * 3.14159265358979323846f * 1.0 * 1000.0f * (float)n / FS);
	      float wave2 = (0.5f)     * 0.65f * cosf(2.0f * 3.14159265358979323846f * 2.0 * 1000.0f * (float)n / FS);
	      float wave3 = (1.0/3.0f) * 0.5f * sinf(2.0f * 3.14159265358979323846f * 3.0 * 1000.0f * (float)n / FS);
	      float wave4 = (1.0/4.0f) * 0.3f * cosf(2.0f * 3.14159265358979323846f * 4.0 * 1000.0f * (float)n / FS);
	      float wave5 = (1.0/5.0f) * 0.1f * sinf(2.0f * 3.14159265358979323846f * 5.0 * 1000.0f * (float)n / FS);

	      input[n]  = wave1+wave2+wave3+wave4+wave5;
	      if(input[n] > Max_Wave) Max_Wave = input[n];
	  }
}




/*------------------------------- fir ---------------------------------*/
/* --- بافرها --- */
#define BLOCK_SIZE N
#define TAPS       48

static const dsp_f32_t fir_coeffs[TAPS] = {
    /* خروجی dsp_resample_design_lp_f32 یا طراحی Offline — برای 100Hz@1kHz */
    0.0001f, 0.0005f, 0.0013f, 0.0028f, 0.0052f, 0.0087f, 0.0135f, 0.0196f,
    0.0270f, 0.0355f, 0.0449f, 0.0548f, 0.0647f, 0.0740f, 0.0822f, 0.0888f,
    0.0934f, 0.0958f, 0.0960f, 0.0939f, 0.0898f, 0.0840f, 0.0768f, 0.0687f,
    0.0601f, 0.0513f, 0.0428f, 0.0348f, 0.0276f, 0.0214f, 0.0162f, 0.0119f,
    0.0085f, 0.0059f, 0.0039f, 0.0025f, 0.0015f, 0.0008f, 0.0004f, 0.0002f,
    0.0001f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f
};

/* حالت FIR — در DTCM (فقط CPU) */
static DSP_PLACE_DTCM dsp_f32_t fir_state[TAPS + BLOCK_SIZE - 1];
static dsp_fir_f32_t fir;

/* خروجی (CPU) */
static DSP_PLACE_DTCM dsp_f32_t fir_out_f32[BLOCK_SIZE];
static volatile uint8_t dma_done = 0;

/* تبدیل 12bit ADC به float نرمال‌شده ±1 */
static inline dsp_f32_t adc_to_float(uint16_t v)
{
    return ((dsp_f32_t)v - Max_Wave) / Max_Wave;
}

void Demo_FIR(){
	t0 = micros_abs();
    if (dsp_fir_init_f32(&fir, fir_coeffs, TAPS, BLOCK_SIZE,DSP_FORM_DIRECT, fir_state, sizeof(fir_state)/4, 0) != DSP_OK) {
        Error_Handler();
    }
    t1 = micros_abs();
    dsp_f32_t in[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) in[i] = adc_to_float(input[i]);
    t2 = micros_abs();
    dsp_fir_process_block_f32(&fir, in, fir_out_f32, BLOCK_SIZE);
    t3 = micros_abs();

    printf("\nDemo_FIR>> F1:%1u us| F2:%1u us| F3:%1u us\n",(t1-t0),(t2-t1),(t3-t2));
}

/*------------------------------- iir ---------------------------------*/

#define ORDER      4

/* خروجی طراحی (Design-Time) — RAM */
static double  sos_f64[ (ORDER/2) * 5 ];
static dsp_f32_t sos_f32[ (ORDER/2) * 5 ];
static dsp_f32_t iir_state[ (ORDER/2) * 2 ];
static DSP_PLACE_DTCM dsp_f32_t iir_out_f32[BLOCK_SIZE];
static dsp_iir_f32_t iir;

void Demo_IIR(){

	t0=0,t2=0,t1=0,t4=0;

	 uint16_t nsec = 0;
	 dsp_design_params_t cfg;
    /* ۱) طراحی Butterworth LP مرتبه 4 — فرکانس قطع 100Hz در 1kHz */
    memset(&cfg, 0, sizeof(cfg));
    cfg.fs = 1000.0;
    cfg.fc = 100.0;
    cfg.type = DSP_FILTER_LP;
    cfg.order = ORDER;
    cfg.pSOS = sos_f64;
    cfg.pNumSections = &nsec;
    if (dsp_design_butterworth(&cfg) != DSP_OK) Error_Handler();

    t0 = micros_abs();
    /* ۲) تبدیل به float32 و راه‌اندازی IIR (SOS / TDF2) */
    dsp_design_sos_f64_to_f32(sos_f64, nsec, sos_f32);
    if (dsp_iir_init_f32(&iir, DSP_TDF2, ORDER, sos_f32, iir_state,sizeof(iir_state)/4, 1) != DSP_OK) Error_Handler();
    t1 = micros_abs();
    for (int i = 0; i < BLOCK_SIZE; i++) {
        dsp_f32_t x = ((dsp_f32_t)input[i] - Max_Wave) / Max_Wave;
        t2 = micros_abs();
        dsp_iir_process_sample_f32(&iir, x, &iir_out_f32[i]);
        t3 = micros_abs();
        t4 += (t3-t2);
    }

    printf("\Demo_IIR>> F1:%1u us| F2:%1u us\n",(t1-t0),t4);

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


  printf("\n Start \n");

  Demo_FIR();
  Demo_IIR();

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
