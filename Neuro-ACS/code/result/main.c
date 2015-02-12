#include "hi_stm32f103.c"

int val2 = 0;
int tim_drt2 = 1000;
char* mstr3;
void vadctask2(void* p) {
	while(1) {
		if(tim_drt2 != 0) {
			val2 = Hi_ADC_Read(ADC_CH_1);
			mstr3 = Hi_Str_Concat("\r", Hi_IntToStr(val2));
			Hi_UART_SendStr(Hi_UART2, mstr3);
			vTaskDelay(tim_drt2);
		}
	}
}


int main(void) {
	
	Hi_QGPIO_Out2(GPIOC, GPIO_Pin_9);
	Hi_UART_Init(Hi_UART2, 1, 0, 9600);
	Hi_ADC_Init(TP_1);
	CreateTask2(vadctask2);
	Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
	Hi_GPIO_On(TP_3_PORT, TP_3_PIN);
	vTaskStartScheduler();
	
	while(1) {
	}
}
