#include "hi_stm32f103.c"

char buf2[17];
int tim_drt2 = 300;
int tgl2 = 0;
void vIButton_2_TimerTask(void* params) {
	while(1) {
		if(tim_drt2 != 0) {
			vTaskDelay(tim_drt2);
			Hi_iButton_Read(TP_1_PORT, TP_1_PIN, buf2, &tgl2, 0);
			if(tgl2 == 1) {
				Hi_UART_SendStr(Hi_UART2, buf2);
			}
		}
	}
}


int main(void) {
	
	Hi_QGPIO_Out2(GPIOC, GPIO_Pin_9);
	Hi_UART_Init(Hi_UART2, 1, 0, 9600);
	xTaskCreate(vIButton_2_TimerTask, (signed char*) "vDS18B20_2_TimerTask", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	Hi_UART_SendStr(Hi_UART2, "Hello\r\n");
	vTaskStartScheduler();
	
	while(1) {
	}
}
