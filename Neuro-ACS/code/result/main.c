#include "hi_stm32f103.c"

int tim_tgl6 = 0;
int tim_drt6 = 1000;
int astp6 = 0;
int astpc6 = 1;
void vTimer6Task(void* params) {
	while(1) {
		if(tim_tgl6) {
			vTaskDelay(tim_drt6);
			Hi_GPIO_Off(TP_4_PORT, TP_4_PIN);
			Hi_GPIO_Off(GPIOA, GPIO_Pin_4);
			if((++astp6 == astpc6) && (astpc6 != 0)) {
				tim_tgl6 = 0;
			}
		}
	}
}
void itc2_method(int index) {
	switch(index) {
		case 0:
			Hi_GPIO_On(TP_4_PORT, TP_4_PIN);
			Hi_GPIO_On(GPIOA, GPIO_Pin_4);
			tim_tgl6 = 1;
			astp6 = 0;
			break;
		case 1:
			break;
	}
}
void vuartlistener1(uint8_t c1) {
	itc2_method(Hi_StrToInt(Hi_CharToStr(c1)));
}
double mlt9 = 0;
int tim_drt9 = 1000;
void vDS18B20_9_StatusHandler(uint8_t s9) {
}
void vDS18B20_9_ResultHandler(double t9) {
	mlt9 = t9;
}
void vDS18B20_9_TimerTask(void* params) {
	while(1) {
		if(tim_drt9 != 0) {
			vTaskDelay(tim_drt9);
			Hi_DS18B20_Scan(TP_5_PORT, TP_5_PIN, vDS18B20_9_StatusHandler, vDS18B20_9_ResultHandler);
		}
	}
}
char* buf10;
int val11 = 0;
int tim_drt11 = 1000;
void vadctask11(void* p) {
	while(1) {
		if(tim_drt11 != 0) {
			val11 = Hi_ADC_Read(ADC_CH_3);
			vTaskDelay(tim_drt11);
		}
	}
}
char sts_tgl12 = 0;
void vinput12(uint8_t s12) {
	if(s12) {
	} else {
	}
}
Hi_QGPIO_In(vinputtask12, vinput12, status12, TP_1_PORT, TP_1_PIN);
int tim_tgl13 = 1;
int tim_drt13 = 1000;
int astp13 = 0;
int astpc13 = 0;
void vTimer13Task(void* params) {
	while(1) {
		if(tim_tgl13) {
			vTaskDelay(tim_drt13);
			buf10 = (char*) pvPortMalloc(128);
			sprintf(buf10, "%s;%s", Hi_DoubleToStr(mlt9), Hi_IntToStr(val11));
			Hi_UART_SendStr(Hi_UART1, buf10);
			vPortFree(buf10);
			if((++astp13 == astpc13) && (astpc13 != 0)) {
				tim_tgl13 = 0;
			}
		}
	}
}


int main(void) {
	
	Hi_QGPIO_Out2(GPIOC, GPIO_Pin_9);
	Hi_UART_Init(Hi_UART1, 1, 0, 9600);
	Hi_GPIO_InitOut(TP_4_PORT, TP_4_PIN, GPIO_Speed_2MHz);
	Hi_GPIO_InitOut(GPIOA, GPIO_Pin_4, GPIO_Speed_2MHz);
	xTaskCreate(vTimer6Task, (signed char*) "vTimer6Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	Hi_UART_AddListener(Hi_UART1, vuartlistener1);
	Hi_GPIO_InitOut(TP_6_PORT, TP_6_PIN, GPIO_Speed_2MHz);
	xTaskCreate(vDS18B20_9_TimerTask, (signed char*) "vDS18B20_9_TimerTask", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	Hi_ADC_Init(TP_3);
	CreateTask2(vadctask11);
	Hi_GPIO_InitIn(TP_1_PORT, TP_1_PIN);
	CreateTask2(vinputtask12);
	xTaskCreate(vTimer13Task, (signed char*) "vTimer13Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	Hi_GPIO_On(TP_6_PORT, TP_6_PIN);
	vTaskStartScheduler();
	
	while(1) {
	}
}
