#include "hi_stm32f103.c"

int tim_tgl3 = 1;
int tim_drt3 = 10;
int astp3 = 0;
int astpc3 = 0;
char* buf4;
void vTimer3Task(void* params) {
  while(1) {
    if(tim_tgl3) {
      vTaskDelay(tim_drt3);
      buf4 = (char*) pvPortMalloc(128);
      sprintf(buf4, "%02.d:%02.d:%02.d ", Hi_RTC_GetHour(), Hi_RTC_GetMin(), Hi_RTC_GetSec());
      Hi_UART_SendStr(Hi_UART1, buf4);
      vPortFree(buf4);
      if((++astp3 == astpc3) && (astpc3 != 0)) {
        tim_tgl3 = 0;
      }
    }
  }
}
char sts_tgl5 = 0;
void vinput5(uint8_t s5) {
  if(s5) {
    NVIC_SystemReset();
  } else {
  }
}
Hi_QGPIO_In(vinputtask5, vinput5, status5, TP_1_PORT, TP_1_PIN);
int tim_tgl6 = 1;
int tim_drt6 = 100;
int astp6 = 0;
int astpc6 = 0;
void vTimer6Task(void* params) {
  while(1) {
    if(tim_tgl6) {
      vTaskDelay(tim_drt6);
      Hi_GPIO_Invert(TP_3_PORT, TP_3_PIN);
      if((++astp6 == astpc6) && (astpc6 != 0)) {
        tim_tgl6 = 0;
      }
    }
  }
}


int main(void) {
  
  Hi_RTC_Init();
  Hi_RTC_SetTime(23, 59, 50);
  Hi_UART_Init(Hi_UART1, 1, 0, 9600);
  xTaskCreate(vTimer3Task, (signed char*) "vTimer3Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_GPIO_InitIn(TP_1_PORT, TP_1_PIN);
  CreateTask2(vinputtask5);
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer6Task, (signed char*) "vTimer6Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_UART_Init(Hi_UART1, 1, 0, 9600);
  Hi_UART_SendStr(Hi_UART1, "Hello\r\n");
  vTaskStartScheduler();
  
  while(1) {
  }
}
