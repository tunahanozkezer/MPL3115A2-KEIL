#include "main.h"
#include "stm32f1xx_hal.h"

#define MPL3115Address	0xC0
I2C_HandleTypeDef hi2c1;

//******************** DEGISKENLER **********************
uint8_t i2cBuf[8],i=0,tempSetting,kalibre_sayac;
float altitude,tempcsb,t_altitude,kalibre_top,kalibre,irtifa;

//******************** FONKSIYONLAR *********************
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void irtifa_config(void);
float irtifa_olc(void);
void kalibrasyon(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
	//Irtifa sensorunun ön ayarini yapan fonksiyon
	irtifa_config();
	
  while(1)
  {
		irtifa = irtifa_olc();
		kalibrasyon();
	}
}

void irtifa_config(void)
{
	//Sensörün altimetresi kullanilacagi belirleniyor
	i2cBuf[0] = 0x26;
	i2cBuf[1] = 0xB8;
	HAL_I2C_Master_Transmit(&hi2c1,MPL3115Address,i2cBuf,2,100);
	//Data flagleri aktif ediliyor
	i2cBuf[0] = 0x13; 
	i2cBuf[1] = 0x07; 
	HAL_I2C_Master_Transmit(&hi2c1,MPL3115Address,i2cBuf,2,100);
	//Sensör aktif ediliyor
	i2cBuf[0] = 0x26;  
	i2cBuf[1] = 0xB9;
	HAL_I2C_Master_Transmit(&hi2c1,MPL3115Address,i2cBuf,2,100);
	
}

float irtifa_olc(void)
{
		//Irtifa okunacagi sensore bildiriliyor
		i2cBuf[0] = 0x01;
		i2cBuf[1] = 0x00;
		HAL_I2C_Master_Transmit(&hi2c1,MPL3115Address,i2cBuf,2,100);
		//Sensörden irtifa verisi isteniyor
		i2cBuf[0] = 0x01;
		HAL_I2C_Master_Receive(&hi2c1, MPL3115Address|0x01, &i2cBuf[0], 6, 10);
		//Sensor irtifa verisini lsb msb ve csb oalrak veriyor alinan bu veri birlesitiliyor
		t_altitude = (i2cBuf[1]<<8 | i2cBuf[2]);
		tempcsb = (i2cBuf[3]>>4)/16.0;
		altitude = (t_altitude + tempcsb)-kalibre;
		return altitude;
}

void kalibrasyon(void)
{
	if(irtifa>0 && kalibre_sayac==0)
{ 
	kalibre = irtifa;kalibre_sayac++;
}
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
