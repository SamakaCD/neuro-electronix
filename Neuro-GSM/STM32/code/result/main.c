#include "hi_stm32f103.c"

int tim_tgl2 = 1;
int tim_drt2 = 100;
int astp2 = 0;
int astpc2 = 0;
void vTimer2Task(void* params) {
  while(1) {
    if(tim_tgl2) {
      vTaskDelay(tim_drt2);
      if((++astp2 == astpc2) && (astpc2 != 0)) {
        tim_tgl2 = 0;
      }
    }
  }
}
int tim_tgl4 = 1;
int tim_drt4 = 100;
int astp4 = 0;
int astpc4 = 0;
void vTimer4Task(void* params) {
  while(1) {
    if(tim_tgl4) {
      vTaskDelay(tim_drt4);
      if((++astp4 == astpc4) && (astpc4 != 0)) {
        tim_tgl4 = 0;
      }
    }
  }
}


int main(void) {
  
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer2Task, (signed char*) "vTimer2Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_QGPIO_Out2(GPIOC, GPIO_Pin_9);
  Hi_UART_Init(Hi_UART1, 1, 1, 9600);
  xTaskCreate(vTimer4Task, (signed char*) "vTimer4Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_GPIO_InitOut(RELAY_PORT, RELAY_PIN, GPIO_Speed_2MHz);
  vTaskStartScheduler();
  
  while(1) {
  }
}
