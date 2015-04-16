#include "hi_lib.h"
#include "hi_ethernet.h"

char* buf2;
void vdatacallback1(u8* buf1, u16 len1) {
  buf2 = (char*) pvPortMalloc(128);
  sprintf(buf2, "You sent %s", buf1);
  Hi_Eth_Send(buf2);
  vPortFree(buf2);
}
void vdhcpcallback1(u8* _ip1) {
}
int tim_tgl3 = 0;
int tim_drt3 = 100;
int astp3 = 0;
int astpc3 = 0;
void vTimer3Task(void* params) {
  while(1) {
    if(tim_tgl3) {
      vTaskDelay(tim_drt3);
      Hi_GPIO_Invert(GPIOC, GPIO_Pin_0);
      if((++astp3 == astpc3) && (astpc3 != 0)) {
        tim_tgl3 = 0;
      }
    }
  }
}


void vHiInitTask(VP) {
  u8* _myip1 = {192, 168, 1, 1};
  u8* _netmask1 = {255, 255, 255, 0};
  u8* _gatewayip1 = {192, 168, 1, 1};
  u8* _dns1 = {8, 8, 8, 8};
  Hi_Eth_AddCallback(vdatacallback1);
  Hi_Eth_AddDHCPIPCallback(vdhcpcallback1);
  Hi_Eth_Init(_myip1, _netmask1, _gatewayip1, _dns1, 80);
  Hi_GPIO_InitOut(GPIOC, GPIO_Pin_0, GPIO_Speed_2MHz);
  xTaskCreate(vTimer3Task, (signed char*) "vTimer3Task", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  tim_tgl3 = 1;
  astp3 = 0;
  vTaskDelete(NULL);
}

int main(void) {
  CreateTask2(vHiInitTask);
  vTaskStartScheduler();
  
  while(1) {
  }
}
