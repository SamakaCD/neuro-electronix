#include "stm32f10x.h"
#include "hi_stm32f103.c"
#include "FreeRTOS.h"
#include "task.h"

char sts_tgl2 = 0;
void vinput2(uint8_t s2) {
  if(s2) {
    Hi_GPIO_On(TP_3_PORT, TP_3_PIN);
  } else {
    Hi_GPIO_Off(TP_3_PORT, TP_3_PIN);
  }
}
Hi_QGPIO_In(vinputtask2, vinput2, status2, TP_1_PORT, TP_1_PIN);


int main(void) {
  
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  Hi_GPIO_InitIn(TP_1_PORT, TP_1_PIN);
  CreateTask2(vinputtask2);
  vTaskStartScheduler();
  
  while(1) {
  }
}
