
#include "main.h"

/

#include "i2c-lcd.h"
#include "stdio.h"


I2C_HandleTypeDef hi2c1;


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);


#define DS3231_ADDRESS 0xD0

uint8_t decToBcd(int val)
{
  return (uint8_t)( (val/10*16) + (val%10) );
}

int bcdToDec(uint8_t val)
{
  return (int)( (val/16*10) + (val%16) );
}

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
} TIME;

TIME time;

// function to set time

void Set_Time (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year)
{
	uint8_t set_time[7];
	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);
	set_time[3] = decToBcd(dow);
	set_time[4] = decToBcd(dom);
	set_time[5] = decToBcd(month);
	set_time[6] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
}

void Get_Time (void)
{
	uint8_t get_time[7];
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000);
	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);
}

float Get_Temp (void)
{
	uint8_t temp[2];

	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x11, 1, temp, 2, 1000);
	return ((temp[0])+(temp[1]>>6)/4.0);
}

void force_temp_conv (void)
{
	uint8_t status=0;
	uint8_t control=0;
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x0F, 1, &status, 1, 100);  // read status register
	if (!(status&0x04))
	{
		HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x0E, 1, &control, 1, 100);  // read control register
		HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x0E, 1, (uint8_t *)(control|(0x20)), 1, 100);
	}
}

float TEMP;
char buffer[15];


int main(void)
{

  HAL_Init();


  SystemClock_Config();


  MX_GPIO_Init();
  MX_I2C1_Init();


  lcd_init ();

  while (1)
  {

	  Get_Time();
	  sprintf (buffer, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);
	  lcd_put_cur (0,0);
	  lcd_send_string(buffer);

	  sprintf (buffer, "%02d-%02d-20%02d", time.dayofmonth, time.month, time.year);
	  lcd_put_cur(1, 0);
	  lcd_send_string(buffer);

	  force_temp_conv();

	  TEMP = Get_Temp();

	  lcd_put_cur(0, 10);

	  sprintf (buffer, "%f", TEMP);

	  lcd_send_string(buffer);

	  HAL_Delay(500);
  }

}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


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
    Error_Handler();
  }

}


static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
