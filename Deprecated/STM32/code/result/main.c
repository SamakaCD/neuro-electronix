#include "hi_stm32f103.c"

int tim_tgl2 = 1;
int tim_drt2 = 100;
int astp2 = 0;
int astpc2 = 0;
void vTimer2Task(void* params) {
  while(1) {
    if(tim_tgl2) {
      vTaskDelay(tim_drt2);
      Hi_GPIO_Invert(TP_3_PORT, TP_3_PIN);
      if((++astp2 == astpc2) && (astpc2 != 0)) {
        tim_tgl2 = 0;
      }
    }
  }
}
double mlt3 = 0;
int tim_drt3 = 0;
void vDS18B20_3_StatusHandler(uint8_t s3) {
}
void vDS18B20_3_ResultHandler(double t3) {
  Hi_UART_SendStr(Hi_UART1, Hi_DoubleToStr(t3));
}
void vDS18B20_3_TimerTask(void* params) {
  while(1) {
    if(tim_drt3 != 0) {
      vTaskDelay(tim_drt3);
      Hi_DS18B20_Scan(TP_2_PORT, TP_2_PIN, vDS18B20_3_StatusHandler, vDS18B20_3_ResultHandler);
    }
    }
  }
int tim_tgl6 = 1;
int tim_drt6 = 1000;
int astp6 = 0;
int astpc6 = 0;
void vTimer6Task(void* params) {
  while(1) {
    if(tim_tgl6) {
      vTaskDelay(tim_drt6);
      Hi_DS18B20_Scan(TP_2_PORT, TP_2_PIN, vDS18B20_3_StatusHandler, vDS18B20_3_ResultHandler);
      if((++astp6 == astpc6) && (astpc6 != 0)) {
        tim_tgl6 = 0;
      }
    }
  }
}


int main(void) {
  
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer2Task, (signed char*) "vTimer2Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_UART_Init(Hi_UART1, 1, 0, 9600);
  xTaskCreate(vDS18B20_3_TimerTask, (signed char*) "vDS18B20_3_TimerTask", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_GPIO_InitOut(TP_1_PORT, TP_1_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer6Task, (signed char*) "vTimer6Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  Hi_GPIO_On(TP_1_PORT, TP_1_PIN);
  vTaskStartScheduler();
  
  while(1) {
  }
}
