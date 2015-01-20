#include "hi_stm32f103.c"

int tim_tgl1 = 1;
int tim_drt1 = 100;
int astp1 = 0;
int astpc1 = 0;
void vTimer1Task(void* params) {
  while(1) {
    if(tim_tgl1) {
      vTaskDelay(tim_drt1);
      Hi_GPIO_Invert(TP_3_PORT, TP_3_PIN);
      if((++astp1 == astpc1) && (astpc1 != 0)) {
        tim_tgl1 = 0;
      }
    }
  }
}


int main(void) {
  
  Hi_GPIO_InitOut(TP_3_PORT, TP_3_PIN, GPIO_Speed_2MHz);
  xTaskCreate(vTimer1Task, (signed char*) "vTimer1Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  
  while(1) {
  }
}
