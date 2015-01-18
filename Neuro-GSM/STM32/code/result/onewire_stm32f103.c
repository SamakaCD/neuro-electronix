#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#define Time_Pulse_Delay_Low	10
#define Time_Pulse_Delay_High	60
#define Time_Reset_Low			480
#define Time_After_Reset		350
#define Time_Hold_Down			2

#define One_Wire_Read_ROM		0x33
#define One_Wire_Skip_ROM		0xCC
#define One_Wire_Search_ROM		0xF0
#define One_Wire_Match_ROM		0x55

#define One_Wire_Success		0x00
#define One_Wire_Error_No_Echo	0x01
#define One_Wire_Bus_Low_Error	0x02
#define One_Wire_Device_Busy	0x03
#define One_Wire_CRC_Error		0x04

#define US_TICKS 2

void delay_one_us() {
	int i;
	for(i = 0; i < US_TICKS; i++) { }
}


void delay_us(u32 us) {
	int i = 0;
	for(i = 0; i < us; i++) {
		delay_one_us();
	}
}


void delay_ms(uint16_t ms) {
	int i, j;
	for(i = 0; i < ms; i++) {
		for(j = 0; j < 1000; j++) {
			delay_one_us();
		}
	}
}


/* ====== GPIO ====== */
void PIN_ON(GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIOx->BSRR=PINx;
	//GPIOx->ODR=GPIOx->IDR|(PINx);
}

void PIN_OFF(GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIOx->BRR=PINx;
	//GPIOx->ODR=GPIOx->IDR&(~(PINx));
}

u8 PIN_SYG(GPIO_TypeDef * GPIOx, u16 PINx)
{
	if((GPIOx->IDR&PINx)!=0)
	{return 1;}
	else
	{return 0;}
}

void PIN_IN (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void PIN_OUT_PP (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void PIN_OUT_OD (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void PIN_INV(GPIO_TypeDef * GPIOx, u16 PINx)
{
	if(PIN_SYG(GPIOx, PINx)!=0)
	{PIN_OFF(GPIOx, PINx);}
	else
	{PIN_ON(GPIOx, PINx);}
}

/* ===== OneWire ===== */

unsigned int One_Wire_Reset(GPIO_TypeDef * GPIOx, u16 PINx)
{
	unsigned int tmp;
	PIN_IN(GPIOx, PINx);
	if ((PIN_SYG(GPIOx, PINx))==0)	return One_Wire_Bus_Low_Error;
	PIN_OUT_PP(GPIOx, PINx);
	PIN_OFF(GPIOx, PINx);
	delay_us(Time_Reset_Low);
	PIN_ON(GPIOx, PINx);
	PIN_IN(GPIOx, PINx);
	delay_us(Time_Pulse_Delay_High);
	if ((PIN_SYG(GPIOx, PINx))==0) tmp=One_Wire_Success;
		else tmp=One_Wire_Error_No_Echo;
	delay_us(Time_After_Reset);
	return tmp;
}

void One_Wire_Write_Bit (unsigned char Bit,GPIO_TypeDef * GPIOx, u16 PINx)
{
	PIN_OUT_PP(GPIOx, PINx);
	PIN_OFF(GPIOx, PINx);
	if (Bit==0)
	{
		delay_us(Time_Pulse_Delay_High);
		PIN_ON(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_Low);
	}
	else
	{
		delay_us(Time_Pulse_Delay_Low);
		PIN_ON(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_High);
	}
	PIN_IN(GPIOx, PINx);
}

unsigned char One_Wire_Read_Bit(GPIO_TypeDef* GPIOx, u16 PINx) {
		unsigned char tmp;
	 	PIN_OUT_PP(GPIOx, PINx);
		PIN_OFF(GPIOx, PINx);
		delay_us(Time_Hold_Down);
		PIN_IN(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_Low);
		if ((PIN_SYG(GPIOx, PINx))!=0)	tmp = 1;
			else tmp = 0;
		delay_us(Time_Pulse_Delay_High);
		return tmp;
}

void One_Wire_Write_Byte(unsigned char Byte,GPIO_TypeDef * GPIOx, u16 PINx)
{
	unsigned char cnt;
	for (cnt=0;cnt!=8;cnt++) One_Wire_Write_Bit(Byte&(1<<cnt),GPIOx, PINx);
}

unsigned char One_Wire_Read_Byte(GPIO_TypeDef * GPIOx, u16 PINx)
{
	unsigned char tmp=0;
	unsigned char cnt;
	for (cnt=0;cnt!=8;cnt++)
		if (One_Wire_Read_Bit(GPIOx, PINx)!=0)	tmp|=(1<<cnt);
	delay_us(Time_Pulse_Delay_High);
	return tmp;
}
