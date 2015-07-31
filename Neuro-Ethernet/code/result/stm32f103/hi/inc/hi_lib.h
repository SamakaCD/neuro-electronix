#define NEURO_ETHERNET
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_iwdg.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "misc.h"
#include "math.h"
#include <stdarg.h>

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

#define TP_1 TP_1_PORT, TP_1_PIN
#define TP_2 TP_2_PORT, TP_2_PIN
#define TP_3 TP_3_PORT, TP_3_PIN
#define TP_4 TP_4_PORT, TP_4_PIN
#define TP_5 TP_5_PORT, TP_5_PIN
#define TP_6 TP_6_PORT, TP_6_PIN

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

#define SYSLED_PORT GPIOC
#define SYSLED_PIN  GPIO_Pin_0
#define SYSLED      GPIOC, GPIO_Pin_0

#define SYSBTN_PORT GPIOC
#define SYSBTN_PIN  GPIO_Pin_13
#define SYSBTN		GPIOC, GPIO_Pin_13

#define WEAK __attribute__ ((weak))
#define MAX(A, B) ((A > B)?A:B)
#define MIN(A, B) ((A < B)?A:B)
#define _FPTR(RET, NAME, PARAM) RET (*NAME) (PARAM)

#define bool	_Bool
#define false	0
#define true	1

#define CreateTask(CODE, NAME, HANDLE) xTaskCreate(CODE, NAME, configMINIMAL_STACK_SIZE, NULL, 1, HANDLE)
#define CreateTask2(CODE) xTaskCreate(CODE, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL)
#define VP void* params

#define FLASH_PAGE_SIZE  2048
#define FLASH_FIRST_ADDR 0x8014000 // Page 40
#define FLASH_PARTITION_PAGE 0x807D000 // Page 250

#define Hi_LED_Mode_1 1
#define Hi_LED_Mode_2 2
#define Hi_LED_Mode_3 3
#define Hi_LED_Mode_4 4
#define Hi_LED_Mode_5 5

#define DBG_UART USART1
#define Hi_UART_BufferSize 256
#define Hi_UART_ListenersCount 256

#define Hi_UART_Mode_W  0
#define Hi_UART_Mode_RW 1

#define Hi_UART1 0
#define Hi_UART2 1
#define Hi_UART3 2
#define Hi_UART4 3
#define Hi_UART5 4

#define Hi_UART_InitPattern(__NAME, PORT, PINRX, PINTX, FUNCNAME) 	\
		Uart = Hi_UARTs[uart]; 										\
		if(echo == 1) { Uart.Echo = echo; }							\
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
			xTaskCreate(FUNCNAME, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL);	\
			Uart.Buffer = (uint8_t*) pvPortMalloc(Hi_UART_BufferSize);	\
			Uart.CharListeners = pvPortMalloc(Hi_UART_ListenersCount); 	\
			Uart.EntListeners = pvPortMalloc(Hi_UART_ListenersCount); 	\
			Uart.BufferCounter = 0;										\
			Uart.BufferSize = Hi_UART_BufferSize;						\
			Uart.Initialised = 1; 										\
			Uart.Def = __NAME;											\
		}																\
		Hi_UARTs[uart] = Uart;											\

#define Hi_QFlash_Start(PADDR) Hi_Flash_Unlock(); Hi_Flash_ErasePage(PADDR); Hi_Flash_StartRecord();
#define Hi_QFlash_Stop() Hi_Flash_EndRecord(); Hi_Flash_Lock();

#define Hi_Proto_PacketType_Std 0b00000000
#define Hi_Proto_PacketType_1   0b01000000
#define Hi_Proto_PacketType_2   0b10000000
#define Hi_Proto_PacketType_3   0b11000000
#define Hi_Proto_BufferMax 21

#define Hi_CM_ListenersMax 32
#define Hi_CM_NONE -1
#define Hi_CM_UART  0
#define Hi_CM_HID   1
#define Hi_CM_CAN   2
#define Hi_CM_GSM   3
#define Hi_CM_ETHERNET 4
#define Hi_CM_UART_Timeout 	100
#define Hi_CM_ResponseOK	200
		
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

typedef struct {
	uint8_t  PacketType;
	uint8_t  MarkerSize;
	uint8_t  DataSize;
	uint32_t Marker;
	char*    Data;
} CM_Result;

void	__println(char* pFormat, ...);
double	iitod(int a, int b);
int		Hi_NumberLen(int n);
void	Hi_CharToHex(char c, char* buf);
void 	Hi_IntToBin(uint8_t c, char* buf);
char*	Hi_TrimZeros(char* target);

char*	Hi_IntToStr(int i);
char*	Hi_LongToStr(long l);
char*	Hi_FloatToStr(float f);
char*	Hi_DoubleToStr(double d);
int		Hi_StrToInt(char* s);
long	Hi_StrToLong(char* s);
float	Hi_StrToFloat(char* s);
double	Hi_StrToDouble(char* s);
double	Hi_StrToDouble(char* s);
char	Hi_StrToChar(char* s);
char*	Hi_CharToStr(char c);
char*	Hi_Str_Concat(char* s1, char* s2);


/* GPIO */
void	Hi_GPIO_InitIn(GPIO_TypeDef* port, int pin);
void	Hi_GPIO_InitOut(GPIO_TypeDef* port, int pin, GPIOSpeed_TypeDef speed);
void	Hi_GPIO_On(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void	Hi_GPIO_Off(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void	Hi_GPIO_Invert(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
u8		Hi_GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void	Hi_ADC_Init(GPIO_TypeDef* port, int pin);
int		Hi_ADC_Read(int channel);


/* === QUICK GPIO FUNCTIONS FOR DEBUG === */

#define Hi_QGPIO_Out(PORT, PIN) Hi_GPIO_InitOut(PORT, PIN, GPIO_Speed_2MHz)
#define Hi_QGPIO_Out2(PORT, PIN) Hi_GPIO_InitOut(PORT, PIN, GPIO_Speed_2MHz);Hi_GPIO_On(PORT, PIN);
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

/* UART */
void	Hi_UART_Send(uint8_t uart, uint8_t data);
void	Hi_UART_SendStr(uint8_t uart, char* string);
void	Hi_UART_Init(uint8_t uart, uint8_t mode, uint8_t echo, uint32_t speed);
void	Hi_UART_AddListener(uint8_t uart, _FPTR(void, listener, uint8_t));
void	Hi_UART_AddEntListener(uint8_t uart, _FPTR(void, listener, uint8_t*));

void	Hi_DS18B20_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, _FPTR(void, ErrorHandler, uint8_t), _FPTR(void, ResultHandler, double));
void 	Hi_iButton_Read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, char* buffer, int* pToggle, uint8_t type);

/* RTC */
void	Hi_RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec);
u32		Hi_RTC_GetTime();
uint8_t	Hi_RTC_GetHour();
uint8_t Hi_RTC_GetMin();
uint8_t Hi_RTC_GetSec();

char*	Hi_Logger_Print(int id, char* key, char* buffer, int* last_record_pos, uint8_t clear);

/* Flash */
void	Hi_Flash_DumpProtocolPage(uint32_t address, char* buffer, uint16_t* dataSize);
void	Hi_Flash_WriteData(uint32_t address, char* data);
void	Hi_Flash_WriteData2(uint32_t address, char* data, uint16_t lenght);
void	Hi_Flash_AddData(uint32_t address, char* data);
void	Hi_Flash_AddData2(uint32_t address, char* data, uint16_t leght);
void	Hi_Flash_CreatePartition(char* name);
void	Hi_Flash_Print(uint32_t address, uint8_t count);

/* NeuroProto */
uint8_t	Hi_Proto_Crc8(char* pcBlock, uint8_t len);
uint8_t Hi_Proto_CheckCRC(char* pcBlock, uint8_t len, uint8_t crc);
uint8_t Hi_Proto_CreateDesriptior(uint8_t packetType, uint8_t markerSize, uint8_t size);
uint8_t Hi_Proto_GetDescriptorPacketType(uint8_t descriptor);
uint8_t Hi_Proto_GetDescriptorMarkerSize(uint8_t descriptor);
uint8_t Hi_Proto_GetDescriptorDataSize(uint8_t descriptor);
uint8_t Hi_Proto_MarkerSize(uint32_t marker);
u8* 	Hi_Proto_CreateMarkerArr(uint32_t marker);
char* 	Hi_Proto_CreatePack(uint8_t packetType, uint32_t markerInt, char* data, uint8_t dataSize, uint8_t* sz);
char* 	Hi_Proto_CreatePack_WO_CRC(uint8_t packetType, uint32_t markerInt, char* data, uint8_t dataSize, uint8_t* sz);
void	Hi_Proto_AllocateBuffer();
void	Hi_Proto_WriteBuffer(char c);
void	Hi_Proto_ClearBuffer();

/* ConnectionManager */
void	Hi_CM_AddListener(_FPTR(void, listener, CM_Result));
void	Hi_CM_Init();
uint8_t Hi_CM_GetConnection();
void	Hi_CM_SendData(char* data, uint8_t len);
uint8_t Hi_CM_IsUARTActive();
void	Hi_CM_SendDataByConnection(char* data, uint8_t connection, uint8_t len);

void	Hi_IWDG_Enable(u16 watchTimeMillis);
void	Hi_ResetInit();
void	Hi_LED_Init();
void	Hi_LED_SetMode(u8 mode);
