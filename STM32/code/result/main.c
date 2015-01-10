#include "hi_stm32f103.c"

char sts_tgl1 = 0;
void vinput1(uint8_t s1) {
  if(s1) {
    NVIC_SystemReset();
  } else {
  }
}
Hi_QGPIO_In(vinputtask1, vinput1, status1, TP_1_PORT, TP_1_PIN);
int tim_tgl4 = 1;
int tim_drt4 = 100;
int astp4 = 0;
int astpc4 = 0;
void vTimer4Task(void* params) {
  while(1) {
    if(tim_tgl4) {
      vTaskDelay(tim_drt4);
      Hi_GPIO_Invert(TP_3_PORT, TP_3_PIN);
      if((++astp4 == astpc4) && (astpc4 != 0)) {
        tim_tgl4 = 0;
      }
    }
  }
}


int main(void) {
  
  Hi_GPIO_InitIn(TP_1_PORT, TP_1_PIN);
  CreateTask2(vinputtask1);
  Hi_UART_Init(Hi_UART2, 1, 1, 9600);
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer4Task, (signed char*) "vTimer4Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  
  while(1) {
  }
}
