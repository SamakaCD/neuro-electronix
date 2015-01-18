#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "onewire_stm32f103.c"
#include "FreeRTOS.h"
#include "misc.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "task.h"
#include "math.h"

#define BUS_GPIOA RCC_APB2Periph_GPIOA
#define BUS_GPIOB RCC_APB2Periph_GPIOB
#define BUS_GPIOC RCC_APB2Periph_GPIOC
#define BUS_GPIOD RCC_APB2Periph_GPIOD
#define BUS_GPIOE RCC_APB2Periph_GPIOE
#define BUS_GPIOF RCC_APB2Periph_GPIOF
#define BUS_GPIOG RCC_APB2Periph_GPIOG

#define EXTI_LINE_0  EXTI_Line0
#define EXTI_LINE_1  EXTI_Line1
#define EXTI_LINE_2  EXTI_Line2
#define EXTI_LINE_3  EXTI_Line3
#define EXTI_LINE_4  EXTI_Line4
#define EXTI_LINE_5  EXTI_Line5
#define EXTI_LINE_9  EXTI_Line9
#define EXTI_LINE_13 EXTI_Line13

#define EXTI_CHANNEL_0  EXTI0_IRQn
#define EXTI_CHANNEL_1  EXTI1_IRQn
#define EXTI_CHANNEL_2  EXTI2_IRQn
#define EXTI_CHANNEL_3  EXTI3_IRQn
#define EXTI_CHANNEL_4  EXTI4_IRQn
#define EXTI_CHANNEL_5  EXTI9_5_IRQn
#define EXTI_CHANNEL_9  EXTI9_5_IRQn
#define EXTI_CHANNEL_13 EXTI15_10_IRQn

#define TIMER_DIVIDER 1000
#define ANTI_BOUNCE_TIME 100

#define RELAY_PORT GPIOA
#define RELAY_PIN GPIO_Pin_4
#define FE_TRANSISTOR_PORT GPIOB
#define FE_TRANSISTOR_PIN GPIO_Pin_13

#define CPORT_1_PORT GPIOB
#define CPORT_2_PORT GPIOC
#define CPORT_3_PORT GPIOC
#define CPORT_4_PORT GPIOA
#define CPORT_5_PORT GPIOA
#define CPORT_6_PORT GPIOA

#define CPORT_1_PIN GPIO_Pin_0
#define CPORT_2_PIN GPIO_Pin_5
#define CPORT_3_PIN GPIO_Pin_4
#define CPORT_4_PIN GPIO_Pin_7
#define CPORT_5_PIN GPIO_Pin_6
#define CPORT_6_PIN GPIO_Pin_5

#define UNB_A_PORT GPIOA
#define UNB_B_PORT GPIOA
#define UNB_A_PIN  GPIO_Pin_9
#define UNB_B_PIN  GPIO_Pin_10

#define ADC_CH_1 8
#define ADC_CH_2 15
#define ADC_CH_3 14
#define ADC_CH_4 7
#define ADC_CH_5 6
#define ADC_CH_6 5

#define SYSLED_PORT GPIOB
#define SYSLED_PIN  GPIO_Pin_5

#define WEAK __attribute__ ((weak))
#define _FPTR(RET, NAME, PARAM) RET (*NAME) (PARAM)

void WEAK Hi_In_Handler(int chanel);

/* Conversion functions */
char* Hi_TrimZeros(char* target) {
	uint8_t i;
	uint8_t len = strlen(target);

	for(i = len-1; i > 0; i--) {
		if((*(target + i)) == '0') {
			*(target + i) = 0;
		} else {
			return target;
		}
	}

	return target;
}

char* Hi_IntToStr(int i) {
	char* str = (char*) pvPortMalloc(16);
	sprintf(str, "%d", i);
	return str;
}

char* Hi_LongToStr(long l) {
	char* str = (char*) pvPortMalloc(32);
	sprintf(str, "%ld", l);
	return str;
}

char* Hi_FloatToStr(float f) {
	char* str = (char*) pvPortMalloc(32);
	sprintf(str, "%f", f);
	return str;
}

char* Hi_DoubleToStr(double d) {
	char* str = (char*) pvPortMalloc(32);
	sprintf(str, "%g", d);
	return str;
}

char* Hi_CharToStr(char c) {
	char* str = (char*) pvPortMalloc(4);
	sprintf(str, "%c", c);
	return str;
}

int Hi_StrToInt(char* s) {
	if(strcmp(s, "") == 0) { return 0; }
	return atoi(s);
}

long Hi_StrToLong(char* s) {
	if(strcmp(s, "") == 0) { return 0; }
	return atol(s);
}

float Hi_StrToFloat(char* s) {
	if(strcmp(s, "") == 0) { return 0; }
	return atoff(s);
}

double Hi_StrToDouble(char* s) {
	if(strcmp(s, "") == 0) { return 0; }
	return atof(s);
}

char Hi_StrToChar(char* s) {
	if(strcmp(s, "") == 0) { return 0; }
	return s[0];
}

/* Strings */
char* Hi_Str_Concat(char* s1, char* s2) {
	char* buf = (char*) pvPortMalloc(strlen(s1) + strlen(s2));
	snprintf(buf, strlen(s1) + strlen(s2), "%s%s", s1, s2);
	return buf;
}

/* GPIO */
int Hi_GPIO_GetBus(GPIO_TypeDef* port) {
	if(port == GPIOA)
		return BUS_GPIOA;
	else if(port == GPIOB)
		return BUS_GPIOB;
	else if(port == GPIOC)
		return BUS_GPIOC;
	else if(port == GPIOD)
		return BUS_GPIOD;
	else if(port == GPIOE)
		return BUS_GPIOE;
	else if(port == GPIOF)
		return BUS_GPIOF;
	else if(port == GPIOG)
		return BUS_GPIOG;
	else return 0;
}

int Hi_EXTI_GetChannelByLine(int line) {
	if(line == EXTI_LINE_0)
		return EXTI_CHANNEL_0;
	else if(line == EXTI_LINE_1)
		return EXTI_CHANNEL_1;
	else if(line == EXTI_LINE_2)
		return EXTI_CHANNEL_2;
	else if(line == EXTI_LINE_3)
		return EXTI_CHANNEL_3;
	else if(line == EXTI_LINE_4)
		return EXTI_CHANNEL_4;
	else if(line == EXTI_LINE_5)
		return EXTI_CHANNEL_5;
	else if(line == EXTI_LINE_9)
		return EXTI_CHANNEL_9;
	else if(line == EXTI_LINE_13)
		return EXTI_CHANNEL_13;
	else return 0;
}

void Hi_GPIO_InitIn(GPIO_TypeDef* port, int pin) {
	RCC_APB2PeriphClockCmd(Hi_GPIO_GetBus(port), ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = pin;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(port, &gpio);
}

void Hi_GPIO_InitOut(GPIO_TypeDef* port, int pin, GPIOSpeed_TypeDef speed) {
	RCC_APB2PeriphClockCmd(Hi_GPIO_GetBus(port), ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = pin;
	gpio.GPIO_Speed = speed;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(port, &gpio);
}

char Hi_GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	uint8_t bitstatus = 0x00;
	if((GPIOx -> ODR & GPIO_Pin) != (uint32_t) Bit_RESET)
		bitstatus = (uint8_t) Bit_SET;
	else bitstatus = (uint8_t) Bit_RESET;
	return (char) bitstatus;
}

void Hi_GPIO_On(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BSRR = GPIO_Pin;
}

void Hi_GPIO_Off(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BRR = GPIO_Pin;
}

void Hi_GPIO_Invert(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->ODR ^= GPIO_Pin;
}

void Hi_GPIO_InitInEXTI(GPIO_TypeDef* port, uint32_t line) {
	RCC->APB2ENR |= Hi_GPIO_GetBus(port);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	EXTI_InitTypeDef  exti;
	exti.EXTI_Line    = line;
	exti.EXTI_Mode    = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	exti.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel                   = Hi_EXTI_GetChannelByLine(line);
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority        = 0x01;
	nvic.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&nvic);
}

#define HI_PORTON(PORT, PIN) Hi_GPIO_InitOut(PORT, PIN, GPIO_Speed_2MHz); Hi_GPIO_On(PORT, PIN);

void Hi_ADC_Init(GPIO_TypeDef* port, int pin) {
	RCC_APB2PeriphClockCmd(Hi_GPIO_GetBus(port), ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = pin;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(port, &gpio);

	ADC_InitTypeDef  a;
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_DeInit(ADC1);

	a.ADC_Mode = ADC_Mode_Independent;
	a.ADC_ScanConvMode = DISABLE;
	a.ADC_ContinuousConvMode = ENABLE;
	a.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	a.ADC_DataAlign = ADC_DataAlign_Right;
	a.ADC_NbrOfChannel = 6;

	ADC_Init(ADC1, &a);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

int Hi_ADC_Read(int channel) {
	  ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_1Cycles5);
	  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	  return ADC_GetConversionValue(ADC1);
}

/* UART */

#define Hi_UART_1 0
#define Hi_UART_2 1
#define Hi_UART_3 2
#define Hi_UART_4 3
#define Hi_UART_5 4

#define Hi_UART_Mode_W  0
#define Hi_UART_Mode_RW 1

/*GPIO_TypeDef* Hi_UART_PinPortsArr[] = {
		GPIOA, GPIOA // USART1
		GPIOA, GPIOA // USART2
		GPIOB, GPIOB // USART3
		GPIOC, GPIOC // USART4
		GPIOC, GPIOD // USART5
};*/

/* UART Handlers */
void WEAK Hi_UART_Byte_Handler(char* str);
void WEAK Hi_UART_Handler(char* str);
uint8_t Hi_UART_Echo = 0;

void Hi_UART_Init(uint8_t mode, uint8_t echo, uint32_t speed) {
	RCC_APB2PeriphClockCmd((RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO), ENABLE);
	Hi_UART_Echo = echo;

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);
	USART_InitStruct.USART_BaudRate = speed;
	USART_InitStruct.USART_Mode = USART_Mode_Tx;
	if(mode == Hi_UART_Mode_RW) {
		USART_InitStruct.USART_Mode |= USART_Mode_Rx;
	}
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);

	if(mode == Hi_UART_Mode_RW) {
		__enable_irq();
		NVIC_EnableIRQ(USART1_IRQn);
		NVIC_SetPriority(USART1_IRQn, 0);
		USART1->CR1 |= USART_CR1_RXNEIE;
	}
}

void Hi_UART_Send(uint8_t data) {
	while(!(USART1 -> SR & USART_SR_TC));
	USART1 -> DR = data;
}

void Hi_UART_SendStr(char* string) {
	uint8_t i=0;
	while(string[i]) {
		Hi_UART_Send(string[i]);
		i++;
	}
}

char Hi_UART_Buffer[1024];
char Hi_UART_Data;
int Hi_UART_Buffer_Cnt = 0;

void USART1_IRQHandler(void) {
	if(USART1 -> SR & USART_SR_RXNE) {
		if(Hi_UART_Echo) {
			USART1 -> DR = USART1 -> DR; // Echo
		}
		Hi_UART_Data = USART1 -> DR;
		Hi_UART_Buffer[Hi_UART_Buffer_Cnt++] = Hi_UART_Data;

		Hi_UART_Byte_Handler(&Hi_UART_Data);

		if(Hi_UART_Data == '\r') {
			Hi_UART_Buffer[Hi_UART_Buffer_Cnt-1] = 0;
			Hi_UART_Handler(Hi_UART_Buffer);
			Hi_UART_Buffer_Cnt = 0;
			memset(Hi_UART_Buffer, 0, sizeof(Hi_UART_Buffer));
		}
	}
}

int Hi_NumberLen(int n) {
	int o, res = 0;
	o = n;
	if(n < 0) {
		n = abs(n);
		res++;
	}

	while(n > 0) {
		n /= 10;
		res++;
	}
	return res;
}

double iitod(int a, int b) {
	return a + b * (pow(10, -Hi_NumberLen(b)));
}

/* DS18B20 */
#include "ds18b20_stm32f103.c"
void Hi_DS18B20_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, _FPTR(void, ErrorHandler, uint8_t), _FPTR(void, ResultHandler, double)) {
	unsigned char Hi_DS18B20_SN[One_Wire_Device_Number_MAX][DS1822_SERIAL_NUM_SIZE];
	unsigned int Hi_DS18B20_Temps[One_Wire_Device_Number_MAX];
	uint8_t Hi_DS18B20_Status = One_Wire_Success;
	uint8_t Hi_DS18B20_Count = 0;

    taskENTER_CRITICAL();
    {
		// Reset
		Hi_DS18B20_Status = One_Wire_Reset(GPIOx, GPIO_Pin);
    }
    taskEXIT_CRITICAL();
	ErrorHandler(Hi_DS18B20_Status);

    taskENTER_CRITICAL();
    {
    	// Scan
    	Hi_DS18B20_Status = DS1822_Search_Rom2(GPIOx, GPIO_Pin, &Hi_DS18B20_Count, &Hi_DS18B20_SN);
    }
    taskEXIT_CRITICAL();
	ErrorHandler(Hi_DS18B20_Status);

    taskENTER_CRITICAL();
    {
    	// Conversion
    	int i;
    	for(i = 0; i < Hi_DS18B20_Count; i++) {
    		Hi_DS18B20_Status = DS1822_Start_Conversion_by_ROM(GPIOx, GPIO_Pin, &(Hi_DS18B20_SN[i]));
    	}
    }
    taskEXIT_CRITICAL();
	ErrorHandler(Hi_DS18B20_Status);

	//Wait 750ms for conversion ready
	vTaskDelay(750);

    taskENTER_CRITICAL();
    {
		// Getting results
		int i;
		for(i = 0; i < Hi_DS18B20_Count; i++) {
			Hi_DS18B20_Status = DS1822_Get_Conversion_Result_by_ROM_CRC(GPIOx, GPIO_Pin, &Hi_DS18B20_SN[i], &Hi_DS18B20_Temps[i]);
		}
    }
	ErrorHandler(Hi_DS18B20_Status);
    taskEXIT_CRITICAL();

    int i;
	for(i = 0; i < Hi_DS18B20_Count; i++) {
		ResultHandler(iitod((Hi_DS18B20_Temps[i] >> 4), (Hi_DS18B20_Temps[i] & 0x0F)));
	}
}
