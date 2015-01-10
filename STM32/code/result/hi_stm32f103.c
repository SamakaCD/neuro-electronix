#include "FreeRTOS.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "onewire_stm32f103.c"
#include "FreeRTOS.h"
#include "stdio.h"
#include "stdlib.h"
#include "semphr.h"
#include "string.h"
#include "misc.h"
#include "math.h"
#include "task.h"
#include "math.h"
#include <stdarg.h>
#include "heap_4.c"

#define Hi_List_Create(LIST, TYPE)					\
	struct LIST##_ItemStruct {						\
		TYPE 		Item; 							\
		uint32_t 	Index;							\
		struct LIST ## _ItemStruct Next;			\
	};												\
	typedef struct LIST##_ItemStruct LIST##_Item;	\
															\
	typedef struct {										\
		uint32_t	Count;									\
		LIST##_Item 	First;								\
		LIST##_Item 	Last;								\
	} LIST##_Def;										\
	LIST##_Def LIST;

#define Hi_List_Add(LIST, ITEM) 				\
	{											\
		(LIST ## _Item) _Item;					\
		_Item.Item = ITEM;						\
		_Item.Index = LIST.Count;				\
		_Item.Next = NULL;						\
												\
		if(LIST.Count == 0) {					\
			LIST.First = _Item;					\
			LIST.Last  = _Item;					\
		} else {								\
			LIST.Last.Next = (_Item);			\
			LIST.Last = _Item;					\
		}										\
		LIST.Count++;							\
	}

#define Hi_List_Get(LIST, IDX)					\
	{											\
		S_ITEM Item = LIST.First;				\
		uint32_t Index;							\
		if(Item == NULL) {						\
			return NULL;						\
		}										\
		do {									\
			Index = Item.Index;					\
			if(Item.Next == NULL) {				\
				return NULL;					\
			}									\
			Item = (Item.Next);					\
		} while(Index != IDX);					\
		return Item.Item;						\
	}


#define BUS_GPIOA RCC_APB2Periph_GPIOA
#define BUS_GPIOB RCC_APB2Periph_GPIOB
#define BUS_GPIOC RCC_APB2Periph_GPIOC
#define BUS_GPIOD RCC_APB2Periph_GPIOD
#define BUS_GPIOE RCC_APB2Periph_GPIOE
#define BUS_GPIOF RCC_APB2Periph_GPIOF
#define BUS_GPIOG RCC_APB2Periph_GPIOG

#define TIMER_DIVIDER 1000
#define ANTI_BOUNCE_TIME 100

#define RELAY_PORT GPIOA
#define RELAY_PIN GPIO_Pin_4
#define FE_TRANSISTOR_PORT GPIOB
#define FE_TRANSISTOR_PIN GPIO_Pin_13

#define TP_1_PORT GPIOB
#define TP_2_PORT GPIOC
#define TP_3_PORT GPIOC
#define TP_4_PORT GPIOA
#define TP_5_PORT GPIOA
#define TP_6_PORT GPIOA

#define TP_1_PIN GPIO_Pin_0
#define TP_2_PIN GPIO_Pin_5
#define TP_3_PIN GPIO_Pin_4
#define TP_4_PIN GPIO_Pin_7
#define TP_5_PIN GPIO_Pin_6
#define TP_6_PIN GPIO_Pin_5

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
#define MAX(A, B) ((A > B)?A:B)
#define MIN(A, B) ((A < B)?A:B)
#define _FPTR(RET, NAME, PARAM) RET (*NAME) (PARAM)

#define CreateTask(CODE, NAME, HANDLE) xTaskCreate(CODE, NAME, configMINIMAL_STACK_SIZE, NULL, 1, HANDLE)
#define CreateTask2(CODE) xTaskCreate(CODE, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL)

#define FLASH_PAGE_SIZE  2048
#define FLASH_FIRST_ADDR 0x8014000 // Page 40
#define FLASH_PARTITION_PAGE 0x807D000 // Page 250

void Hi_UART_Send1(uint8_t data) {
	while(!(USART1 -> SR & USART_SR_TC));
	USART1 -> DR = data;
}

void Hi_UART_SendStr1(char* string) {
	uint8_t i=0;
	while(string[i]) {
		Hi_UART_Send1(string[i]);
		i++;
	}
}

void __println(char* pFormat, ...) {
	char* _mbuffer = (char*) pvPortMalloc(32);
    va_list ap;

    va_start(ap, pFormat);
    vsprintf(_mbuffer, pFormat, ap);
    va_end(ap);
    Hi_UART_SendStr1(_mbuffer);
    Hi_UART_SendStr1("\r\n");
    vPortFree(_mbuffer);
}

/* Conversion functions */
char* Hi_TrimZeros(char* target) {
	uint8_t i, len = strlen(target);

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

char* Hi_CharToStr(char c) {
	char* buf = (char*) pvPortMalloc(2);
	buf[0] = c;
	buf[1] = 0;
	return buf;
}

/* Strings */
char* Hi_Str_Concat(char* s1, char* s2) {
	char* buf = (char*) pvPortMalloc(strlen(s1) + strlen(s2) + 1);
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

/* === QUICK FUNCTIONS FOR DEBUG === */

#define Hi_QGPIO_Out(PORT, PIN) Hi_GPIO_InitOut(PORT, PIN, GPIO_Speed_2MHz)
#define HI_PORTON(PORT, PIN) Hi_GPIO_InitOut(PORT, PIN, GPIO_Speed_2MHz); Hi_GPIO_On(PORT, PIN);
#define Hi_QGPIO_Out2(PORT, PIN) HI_PORTON(PORT, PIN);

#define Hi_QGPIO_In(FUNC_NAME, FUNC, VAR_STATUS, PORT, PIN) \
uint8_t VAR_STATUS = 1; \
void FUNC_NAME(void* params) { \
	while(1) { \
		if(GPIO_ReadInputDataBit(PORT, PIN) != VAR_STATUS) { \
			VAR_STATUS = !VAR_STATUS; \
			FUNC(!VAR_STATUS); \
			vTaskDelay(ANTI_BOUNCE_TIME); \
		} \
	} \
}

#define Hi_QGPIO_In2(FUNC, PORT, PIN) Hi_QGPIO_In(__QGPIO_Func, FUNC, __QGPIO_Status, PORT, PIN)
#define Hi_QGPIO_In3(PORT, PIN) Hi_GPIO_InitIn(PORT, PIN); CreateTask(__QGPIO_Func, "__QGPIO_Func", NULL);
#define Hi_QResetF() \
void __QResetFunc(uint8_t status) { \
	if(status == 0) NVIC_SystemReset(); \
} \
Hi_QGPIO_In2(__QResetFunc, TP_1_PORT, TP_1_PIN);
#define Hi_QResetI() Hi_QGPIO_In3(TP_1_PORT, TP_1_PIN)

/* ================================= */

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
#define Hi_UART_BufferSize 256
#define Hi_UART_ListenersCount 256

#define Hi_UART_Mode_W  0
#define Hi_UART_Mode_RW 1

#define Hi_UART1 0
#define Hi_UART2 1
#define Hi_UART3 2
#define Hi_UART4 3
#define Hi_UART5 4

typedef struct {
	_FPTR(void, *CharListeners, uint8_t);
	_FPTR(void, *EntListeners, uint8_t*);
	uint8_t CharListenersCount;
	uint8_t EntListenersCount;
	uint8_t  Initialised;
	uint8_t* Buffer;
	uint16_t BufferSize;
	uint16_t BufferCounter;
	uint8_t  Char;
	uint8_t Echo;
	USART_TypeDef* Def;
} Hi_UART_Struct;

Hi_UART_Struct Hi_UARTs[5];

void Hi_UART_Send(uint8_t uart, uint8_t data) {
	while(!(Hi_UARTs[uart].Def -> SR & USART_SR_TC)) { }
	Hi_UARTs[uart].Def -> DR = data;
}

void Hi_UART_SendStr(uint8_t uart, char* string) {
	uint8_t i = 0;
	while(string[i]) {
		__asm("NOP");
		__asm("NOP");
		__asm("NOP");
		Hi_UART_Send(uart, string[i]);
		__asm("NOP");
		__asm("NOP");
		__asm("NOP");
		i++;
	}
}

#define Hi_UART_ListenerPattern(__UARTNAME, __UARTIDX) 			\
		if(__UARTNAME -> SR & USART_SR_RXNE) {					\
			Hi_UART_Struct Uart = Hi_UARTs[__UARTIDX];			\
																\
			if(Uart.Echo) {										\
				__UARTNAME -> DR = __UARTNAME -> DR;			\
			}													\
			Uart.Char = __UARTNAME -> DR;						\
			Uart.Buffer[Uart.BufferCounter++] = Uart.Char;		\
																\
			uint8_t i;											\
			for(i = 0; i < Uart.CharListenersCount; i++) {		\
				Uart.CharListeners[i](Uart.Char);				\
			}													\
																\
			if(Uart.Char == '\r') {								\
				Hi_UART_SendStr(__UARTIDX, "\r\n");				\
				Uart.Buffer[Uart.BufferCounter-1] = 0;			\
				for(i = 0; i < Uart.EntListenersCount; i++) {	\
					Uart.EntListeners[i](Uart.Buffer);			\
				}												\
				memset(Uart.Buffer, 0, Uart.BufferCounter);		\
				Uart.BufferCounter = 0;							\
			}													\
			Hi_UARTs[__UARTIDX] = Uart;							\
		}														\

#define Hi_UART_InitPattern(__NAME, PORT, PINRX, PINTX, FUNCNAME) 	\
		Uart = Hi_UARTs[uart]; 										\
		if(!Uart.Initialised) {										\
			GPIO_InitStruct.GPIO_Pin = PINRX; 						\
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;			\
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; 			\
			GPIO_Init(PORT, &GPIO_InitStruct); 						\
			 	 	 	 	 	 	 	 	 	 	 	 	 	 	\
			GPIO_InitStruct.GPIO_Pin = PINTX; 						\
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		\
			GPIO_Init(PORT, &GPIO_InitStruct); 						\
			 	 	 	 	 	 	 	 	 	 	 	 	 	 	\
			USART_StructInit(&USART_InitStruct); 					\
			USART_InitStruct.USART_BaudRate = speed; 				\
			USART_InitStruct.USART_Mode = USART_Mode_Tx; 			\
			if(mode == Hi_UART_Mode_RW) { 							\
				USART_InitStruct.USART_Mode |= USART_Mode_Rx; 		\
			} 														\
			USART_Init(__NAME, &USART_InitStruct); 					\
			USART_Cmd(__NAME, ENABLE);								\
			CreateTask(FUNCNAME, "", NULL); 						\
			Uart.Buffer = (uint8_t*) pvPortMalloc(Hi_UART_BufferSize);	\
			Uart.CharListeners = pvPortMalloc(Hi_UART_ListenersCount); 	\
			Uart.EntListeners = pvPortMalloc(Hi_UART_ListenersCount); 	\
			Uart.BufferCounter = 0;										\
			Uart.BufferSize = Hi_UART_BufferSize;						\
			Uart.Echo = echo;											\
			Uart.Initialised = 1; 										\
			Uart.Def = __NAME;											\
			Hi_UARTs[uart] = Uart;										\
		}																\

void Hi_UART1_ListenerTask(void* params) {
	while(1) {
		Hi_UART_ListenerPattern(USART1, Hi_UART1);
	}
}

void Hi_UART2_ListenerTask(void* params) {
	while(1) {
		Hi_UART_ListenerPattern(USART2, Hi_UART2);
	}
}

void Hi_UART3_ListenerTask(void* params) {
	while(1) {
		Hi_UART_ListenerPattern(USART3, Hi_UART3);
	}
}

void Hi_UART4_ListenerTask(void* params) {
	while(1) {
		Hi_UART_ListenerPattern(UART4, Hi_UART4);
	}
}

void Hi_UART5_ListenerTask(void* params) {
	while(1) {
		Hi_UART_ListenerPattern(UART5, Hi_UART5);
	}
}

void Hi_UART_Init(uint8_t uart, uint8_t mode, uint8_t echo, uint32_t speed) {
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	Hi_UART_Struct Uart;

	switch(uart) {
		case Hi_UART1:
			RCC_APB2PeriphClockCmd((RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO), ENABLE);
			Hi_UART_InitPattern(USART1, GPIOA, GPIO_Pin_9, GPIO_Pin_10, Hi_UART1_ListenerTask);
			break;
		case Hi_UART2:
			RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO), ENABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			Hi_UART_InitPattern(USART2, GPIOA, GPIO_Pin_2, GPIO_Pin_3, Hi_UART2_ListenerTask);
			break;
		case Hi_UART3:
			RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO), ENABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
			Hi_UART_InitPattern(USART3, GPIOB, GPIO_Pin_10, GPIO_Pin_11, Hi_UART3_ListenerTask);
			break;
		case Hi_UART4:
			RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO), ENABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
			Hi_UART_InitPattern(UART4, GPIOC, GPIO_Pin_10, GPIO_Pin_11, Hi_UART4_ListenerTask);
			break;
		case Hi_UART5:
			RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO), ENABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
			Hi_UART_InitPattern(UART5, GPIOC, GPIO_Pin_12, GPIO_Pin_12, Hi_UART5_ListenerTask);
			break;
	}
}

void Hi_UART_AddListener(uint8_t uart, _FPTR(void, listener, uint8_t)) {
	Hi_UART_Struct Uart = Hi_UARTs[uart];
	Uart.CharListeners[Uart.CharListenersCount++] = listener;
	Hi_UARTs[uart] = Uart;
}

void Hi_UART_AddEntListener(uint8_t uart, _FPTR(void, listener, uint8_t*)) {
	Hi_UART_Struct Uart = Hi_UARTs[uart];
	Uart.EntListeners[Uart.EntListenersCount++] = listener;
	Hi_UARTs[uart] = Uart;
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

void __print(char* pFormat, ...) {
	char* _mbuffer = (char*) pvPortMalloc(256);
    va_list ap;

    va_start(ap, pFormat);
    vsprintf(_mbuffer, pFormat, ap);
    va_end(ap);
    Hi_UART_SendStr1(_mbuffer);
    vPortFree(_mbuffer);
}

void Hi_CharToHex(char c, char* buf) {
	static char hex_symbols[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	uint8_t i = 1;

	do {
		*(buf + i) = hex_symbols[(c % 16)];
		c /= 16;
		i--;
	} while(c != 0);

	if(i == 0) {
		*buf = '0';
	}
}

void Hi_IntToBin(uint8_t c, char* buf) {
	static char symbols[] = { '0', '1' };
	uint8_t i = 1;

	do {
		*(buf + i) = symbols[(c % 2)];
		c /= 2;
		i--;
	} while(c != 0);

	if(i == 0) {
		*buf = '0';
	}
}

/* iButton */
void Hi_iButton_Read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, char* buffer, int* pToggle, uint8_t type) {
	uint8_t i;
	char* b = pvPortMalloc(8);
	char* hex = pvPortMalloc(2);

    taskENTER_CRITICAL();
    {
    	Hi_GPIO_InitOut(GPIOx, GPIO_Pin, GPIO_Speed_50MHz);
    	Hi_GPIO_On(GPIOx, GPIO_Pin);
    	if(One_Wire_Reset(GPIOx, GPIO_Pin) == 0) {
    		(*pToggle)++;
        	One_Wire_Write_Byte(0x33, GPIOx, GPIO_Pin);
        	for(i = 0; i < 8; i++) {
        		b[i] = One_Wire_Read_Byte(GPIOx, GPIO_Pin);
        	}
    	} else {
    		*pToggle = 0;
    	}
    }
    taskEXIT_CRITICAL();

    if(*pToggle == 1) {
        if(type) {
        	for(i = 0; i < 8; i++) {
        		*(buffer+i) = *(b+i);
        	}
        	*(buffer+8) = 0;
        } else {
        	for(i = 0; i < 16; i += 2) {
        		Hi_CharToHex(*(b+i/2), hex);
        		*(buffer+i) = hex[0];
        		*(buffer+i+1) = hex[1];
        	}
        	*(buffer+16) = 0;
        }
    }

    vPortFree(b);
    vPortFree(hex);
}

/* RTC (Temporaty implementation) */

uint32_t Hi_RTC_TimeCnt = 0;
void vHi_RTC_Task(void* params) {
	while(1) {
		if(++Hi_RTC_TimeCnt >= 86400) {
			Hi_RTC_TimeCnt = 0;
		}
		vTaskDelay(1000);
	}
}

uint8_t Hi_RTC_Initialised;
void Hi_RTC_Init() {
	if(!Hi_RTC_Initialised) {
		Hi_RTC_Initialised = 1;
		xTaskCreate(vHi_RTC_Task, "vHi_RTC_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	}
}

void Hi_RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec) {
	if(hour < 24 && min < 60 && sec < 60) {
		Hi_RTC_TimeCnt = 3600 * hour + 60 * min + sec;
	}
}

uint8_t Hi_RTC_GetHour() {
	return Hi_RTC_TimeCnt / 3600;
}

uint8_t Hi_RTC_GetMin() {
	return (Hi_RTC_TimeCnt % 3600) / 60;
}

uint8_t Hi_RTC_GetSec() {
	return (Hi_RTC_TimeCnt % 3600) % 60;
}

/* Logger */

char* Hi_Logger_Print(int id, char* key, char* buffer, int* last_record_pos, uint8_t clear) {
	char* buf = pvPortMalloc(56);
	uint16_t len = sprintf(buf, "%d\t%d:%d:%d\t%s\r\n", id, Hi_RTC_GetHour(), Hi_RTC_GetMin(), Hi_RTC_GetSec(), key);

	uint16_t i;
	for(i = 0; i < len; i++) {
		*(buffer + *last_record_pos + i) = *(buf + i);
	}

	*(last_record_pos) += len;
	if(clear) {
		vPortFree(buf);
	}

	return buf;
}

/* Flash */
#include "flash_stm32f103.c"

void Hi_Flash_DumpProtocolPage(uint32_t address, char* buffer, uint16_t* dataSize) {
	uint16_t i, size = Hi_Flash_Read16(address);
	if(dataSize != NULL) {
		*(dataSize) = size;
	}
	address += 2;
	for(i = 0; i < size; i++) {
		*(buffer + i) = Hi_Flash_Read1(address + i);
		if(i != size) {
			*(buffer + i + 1) = Hi_Flash_Read2(address + i);
			i++;
		}
	}
}

void Hi_Flash_WriteData(uint32_t address, char* data) {
	Hi_Flash_Unlock();
	Hi_Flash_ErasePage(address);
	Hi_Flash_StartRecord();

	Hi_Flash_Write16(address, strlen(data));
	address += 2;
	int i;
	for(i = 0; i < strlen(data); i++) {
		if(i != strlen(data)-1) {
			Hi_Flash_Write((address + i), *(data + i), *(data + i + 1));
			i++;
		} else {
			Hi_Flash_Write((address + i), *(data + i), 0xFF);
		}
	}

	Hi_Flash_EndRecord();
	Hi_Flash_Lock();
}

#define Hi_QFlash_Start(PADDR) Hi_Flash_Unlock(); Hi_Flash_ErasePage(PADDR); Hi_Flash_StartRecord();
#define Hi_QFlash_Stop() Hi_Flash_EndRecord(); Hi_Flash_Lock();

void Hi_Flash_AddData(uint32_t address, char* data) {
	uint16_t i, size = Hi_Flash_Read16(address);

	if(size == 0xFFFF) {
		Hi_Flash_WriteData(address, data);
	} else {
		char* buffer = pvPortMalloc(FLASH_PAGE_SIZE);
		Hi_Flash_DumpProtocolPage(address, buffer, NULL);

		for(i = 0; i < size + strlen(data); i++) {
			*(buffer + size + i) = *(data + i);
		}

		Hi_Flash_Unlock();
		Hi_Flash_ErasePage(address);
		Hi_Flash_StartRecord();

		Hi_Flash_Write16(address, strlen(buffer));
		address += 2;
		for(i = 0; i < strlen(buffer); i++) {
			if(i != strlen(buffer)-1) {
				Hi_Flash_Write((address + i), *(buffer + i), *(buffer + i + 1));
				i++;
			} else {
				Hi_Flash_Write((address + i), *(data + i), 0xFF);
			}
		}

		Hi_Flash_EndRecord();
		Hi_Flash_Lock();
		vPortFree(buffer);
	}
}

void Hi_Flash_WriteData2(uint32_t address, char* data, uint16_t lenght) {
	taskENTER_CRITICAL(); {
		Hi_Flash_Unlock();
		Hi_Flash_ErasePage(address);
		Hi_Flash_StartRecord();
		Hi_Flash_Write16(address, lenght);
		address += 2;
		int i;
		for(i = 0; i < lenght; i++) {
			if(i != lenght-1) {
				Hi_Flash_Write((address + i), *(data + i), *(data + i + 1));
				i++;
			} else {
				Hi_Flash_Write((address + i), *(data + i), 0xFF);
			}
		}

		Hi_Flash_EndRecord();
		Hi_Flash_Lock();
	} taskEXIT_CRITICAL();
}

void Hi_Flash_AddData2(uint32_t address, char* data, uint16_t leght) {
	uint16_t i, size = Hi_Flash_Read16(address);
	if(size == 0xFFFF) {
		Hi_Flash_WriteData2(address, data, leght);
	} else {
		char* buffer = pvPortMalloc(FLASH_PAGE_SIZE);
		Hi_Flash_DumpProtocolPage(address, buffer, NULL);

		for(i = 0; i < size + leght; i++) {
			*(buffer + size + i) = *(data + i);
		}

		taskENTER_CRITICAL(); {
			Hi_Flash_Unlock();
			Hi_Flash_ErasePage(address);
			Hi_Flash_StartRecord();

			Hi_Flash_Write16(address, size + leght);
			address += 2;
			for(i = 0; i < size + leght; i++) {
				if(i != size + leght-1) {
					Hi_Flash_Write((address + i), *(buffer + i), *(buffer + i + 1));
					i++;
				} else {
					Hi_Flash_Write((address + i), *(data + i), 0xFF);
				}
			}

			Hi_Flash_EndRecord();
			Hi_Flash_Lock();
		} taskEXIT_CRITICAL();
		vPortFree(buffer);
	}
}

// Partition page: [L][L]..........[\0]..........[\0]
void Hi_Flash_CreatePartition(char* name) {
	uint8_t j, hasSuchPartition = 0;
	uint16_t i, dataSize = Hi_Flash_Read16(FLASH_PARTITION_PAGE);
	if(dataSize == 0xFFFF) { // We are first here
		Hi_Flash_AddData2(FLASH_PARTITION_PAGE, name, strlen(name)+1);
	} else {
		char* dump = (char*) pvPortMalloc(dataSize);
		char* str = (char*) pvPortMalloc(32);
		Hi_Flash_DumpProtocolPage(FLASH_PARTITION_PAGE, dump, NULL);
		for(i = 0; i < dataSize; i++) {
			if(dump[i]) {
				str[i] = dump[i];
			} else {
				hasSuchPartition = (strcmp(str, name) == 0);
				for(j = 0; j < 32; j++) str[j] = 0;
				break;
			}
		}
		if(!hasSuchPartition)
			Hi_Flash_AddData2(FLASH_PARTITION_PAGE, name, strlen(name)+1);
		vPortFree(dump);
		vPortFree(str);
	}
}



void Hi_Flash_Print(uint32_t address, uint8_t count) {
	uint8_t i, j, c;
	Hi_UART_SendStr1("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n-----------------------------------------------\r\n");
	for(i = 0; i < count; i += 2) {
		__print("%02X %02X ", Hi_Flash_Read1(address + i), Hi_Flash_Read1(address + i + 1));

		if(((i+2) % 16) == 0) {
			__print("\t");
			for(j = 0; j < 16; j++) {
				c = Hi_Flash_Read1(address + j + i - 14);
				Hi_UART_Send1(( (c >= 32) && (c <= 127) ) ? c : '.');
			}
			__print("\r\n");
		}
	}
}

/* NeuroProto */

#define Hi_Proto_PacketType_Std 0b00000000
#define Hi_Proto_PacketType_1   0b01000000
#define Hi_Proto_PacketType_2   0b10000000
#define Hi_Proto_PacketType_3   0b11000000

uint8_t Hi_Proto_Crc8(char* pcBlock, uint8_t len) {
	uint8_t i, j, byte, crc = 0;
	for(i = 0; i < len; i++) {
		byte = pcBlock[i];
		for(j = 0; j < 8; j++) {
			if((byte ^ crc) & 1)
				crc = ((crc ^ 24) >> 1) | 128;
			else crc >>= 1;
			byte >>= 1;
		}
	}
	return crc;
}

uint8_t Hi_Proto_CheckCRC(char* pcBlock, uint8_t len, uint8_t crc) {
	return Hi_Proto_Crc8(pcBlock, len) == crc;
}

uint8_t Hi_Proto_CreateDesriptior(uint8_t packetType, uint8_t markerSize, uint8_t size) {
	return packetType | (markerSize << 4) | MIN(size, 15);
}

uint8_t Hi_Proto_GetDescriptorPacketType(uint8_t descriptor) {
	switch(descriptor >> 6) {
		case 1: return Hi_Proto_PacketType_1;
		case 2: return Hi_Proto_PacketType_2;
		case 3: return Hi_Proto_PacketType_3;
		default: return Hi_Proto_PacketType_Std;
	}
}

uint8_t Hi_Proto_GetDescriptorMarkerSize(uint8_t descriptor) {
	return (descriptor & 48) >> 8;
}

uint8_t Hi_Proto_GetDescriptorDataSize(uint8_t descriptor) {
	return (descriptor & 0b00001111);
}

uint8_t Hi_Proto_MarkerSize(uint32_t marker) {
	if(marker == 0)
		return 0;
	else if(marker <= 0xFF)
		return 1;
	else if(marker <= 0xFFFF)
		return 2;
	else return 3;
}

uint8_t* Hi_Proto_CreateMarkerArr(uint32_t marker) {
	uint8_t i, msz = Hi_Proto_MarkerSize(marker);
	uint8_t* m = (uint8_t*) pvPortMalloc(msz);

	for(i = 1; i <= msz; i++) {
		m[msz - i] = (marker & 0xFF);
		marker >>= 8;
	}
	return m;
}

char* Hi_Proto_CreatePack(uint8_t packetType, uint32_t markerInt, char* data, uint8_t dataSize, uint8_t* sz) {
	uint8_t i, markerSize = Hi_Proto_MarkerSize(markerInt);
	char* packetBuf = (char*) pvPortMalloc(markerSize + dataSize + 2);
	uint8_t* marker = Hi_Proto_CreateMarkerArr(markerInt);
	*(packetBuf) = Hi_Proto_CreateDesriptior(packetType, markerSize, dataSize);

	for(i = 0; i < markerSize; i++) {
		*(packetBuf + i + 1) = *(marker + i);
	}

	for(i = 0; i < dataSize; i++) {
		*(packetBuf + markerSize + i + 1) = *(data + i);
	}

	*(packetBuf + markerSize + dataSize + 1) = Hi_Proto_Crc8(packetBuf, (markerSize + dataSize + 1));
	if(sz != 0) {
		*sz = markerSize + dataSize + 2;
	}
	return packetBuf;
}

#define Hi_Proto_BufferMax 21
char* Hi_Proto_Buffer;
uint8_t Hi_Proto_BufferPos = 0;

void Hi_Proto_AllocateBuffer() {
	Hi_Proto_Buffer = (char*) pvPortMalloc(Hi_Proto_BufferMax);
}

void Hi_Proto_WriteBuffer(char c) {
	Hi_Proto_Buffer[Hi_Proto_BufferPos++] = c;
}

void Hi_Proto_ClearBuffer() {
	vPortFree(Hi_Proto_Buffer);
	Hi_Proto_BufferPos = 0;
}

/* ConnectionManager */

#define Hi_CM_ListenersMax 32
#define Hi_CM_NONE -1
#define Hi_CM_UART  0
#define Hi_CM_HID   1
#define Hi_CM_CAN   2
#define Hi_CM_GSM   3
#define Hi_CM_UART_Timeout 100
#define Hi_CM_ResponseOK 200

typedef struct {
	uint8_t  PacketType;
	uint8_t  MarkerSize;
	uint8_t  DataSize;
	uint32_t Marker;
	char*    Data;
} CM_Result;

uint8_t Hi_CM_IsPacketTransition = 0;
uint8_t Hi_CM_ReceivedCount      = 0;
uint8_t Hi_CM_PacketSize         = 0;
uint8_t Hi_CM_ListenersPos = 0;
volatile uint8_t Hi_CM_WaitingOnUART = 0;
_FPTR(void, *Hi_CM_Listeners, CM_Result);
CM_Result Hi_CM_Result;
xSemaphoreHandle Hi_CM_UARTSemaphore;

void Hi_CM_UARTConnectionListener(uint8_t c) {
	if(Hi_CM_IsPacketTransition) {
		if(Hi_CM_ReceivedCount < (Hi_CM_PacketSize-1)) { // Transition process
			Hi_Proto_WriteBuffer(c);
		} else { // End of transition
			Hi_Proto_WriteBuffer(c); // CRC
			uint8_t i;
			for(i = 1; i < Hi_CM_Result.MarkerSize; i++)
				Hi_CM_Result.Marker = (Hi_CM_Result.Marker << 8) | Hi_Proto_Buffer[i];

			char* data = (char*) pvPortMalloc(Hi_CM_Result.DataSize);
			for(i = 0; i < Hi_CM_Result.DataSize; i++)
				data[i] = Hi_Proto_Buffer[i + Hi_CM_Result.MarkerSize + 1];
			Hi_CM_Result.Data = data;

			if(Hi_Proto_CheckCRC(Hi_Proto_Buffer, Hi_CM_Result.MarkerSize + Hi_CM_Result.DataSize + 1, c))
				for(i = 0; i < Hi_CM_ListenersPos; i++)
					(*(Hi_CM_Listeners + i))(Hi_CM_Result);

			Hi_CM_IsPacketTransition = 0;
			Hi_CM_ReceivedCount      = 0;
			Hi_CM_PacketSize         = 0;
		}
	} else { // Start of transition
		Hi_CM_IsPacketTransition = 1;
		Hi_CM_ReceivedCount = 1;
		Hi_Proto_ClearBuffer();
		Hi_Proto_AllocateBuffer();
		Hi_Proto_WriteBuffer(c);
		Hi_CM_Result.PacketType = Hi_Proto_GetDescriptorPacketType(c);
		Hi_CM_Result.MarkerSize = Hi_Proto_GetDescriptorMarkerSize(c);
		Hi_CM_Result.DataSize = Hi_Proto_GetDescriptorDataSize(c);
		Hi_CM_PacketSize = Hi_CM_Result.MarkerSize + Hi_CM_Result.DataSize + 2;
	}
}

void Hi_CM_UARTListener(uint8_t in) {
	if(Hi_CM_WaitingOnUART && in == Hi_CM_ResponseOK) {
		Hi_CM_WaitingOnUART = 0;
		xSemaphoreGive(Hi_CM_UARTSemaphore);
	}
}

void Hi_CM_AddListener(_FPTR(void, listener, CM_Result)) {
	Hi_CM_Listeners[Hi_CM_ListenersPos++] = listener;
}

void Hi_CM_Init() {
	Hi_UART_AddListener(Hi_UART1, Hi_CM_UARTConnectionListener);
	Hi_UART_AddListener(Hi_UART1, Hi_CM_UARTListener);
	Hi_CM_Listeners = pvPortMalloc(Hi_CM_ListenersMax * sizeof(Hi_CM_Listeners));
	vSemaphoreCreateBinary(Hi_CM_UARTSemaphore);
	xSemaphoreTake(Hi_CM_UARTSemaphore, 1);
}

uint8_t Hi_CM_IsUARTActive() {
	Hi_UART_SendStr1("CHECK_UART");
	Hi_CM_WaitingOnUART = 1;
	if(xSemaphoreTake(Hi_CM_UARTSemaphore, Hi_CM_UART_Timeout)) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t Hi_CM_GetConnection() {
	return Hi_CM_UART;
}

void Hi_CM_SendDataByConnection(char* data, uint8_t connection, uint8_t len) {
	switch(connection) {
		case Hi_CM_UART: {
			uint8_t i;
			for(i = 0; i < len; i++)
				Hi_UART_Send1(data[i]);
			break;
		}
	}
}

void Hi_CM_SendData(char* data, uint8_t len) {
	Hi_CM_SendDataByConnection(data, Hi_CM_GetConnection(), len);
}

/* SharedMemory */

uint16_t* Hi_SM_Markers;

void Hi_SM_Init() {
	Hi_SM_Markers = pvPortMalloc(256);
}

void Hi_SM_Create(uint32_t address, uint16_t len, uint16_t marker) {

}

uint32_t Hi_Logger_LastTime = 0;
uint8_t* Hi_Logger_CreatePack(uint16_t address) {
	uint8_t* pack = (uint8_t*) pvPortMalloc(4);
	uint16_t timeDelta = Hi_RTC_TimeCnt - Hi_Logger_LastTime;
	pack[0] = (address >> 4) & 0xFF;
	pack[1] = address & 0xFF;
	pack[2] = (timeDelta >> 4) & 0xFF;
	pack[3] = timeDelta & 0xFF;
	return pack;
}

uint8_t Hi_Logger_LastTimeAvilable = 0;
void Hi_Logger_Notify(uint16_t address, uint32_t marker) {
	Hi_RTC_Init();
	uint8_t* pack = Hi_Logger_CreatePack(address);
	uint8_t isUARTAvilable = Hi_CM_IsUARTActive();
	if(isUARTAvilable) {
		Hi_UART_SendStr1((char*) pack);
		Hi_Logger_LastTime = Hi_RTC_TimeCnt;
		Hi_Logger_LastTimeAvilable = 1;
	} else { // UART is not avilable. Let's write to flash

		Hi_Logger_LastTimeAvilable = 0;
	}
	vPortFree(pack);
}
