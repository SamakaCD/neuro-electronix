#include "hi_lib.h"
#include "hi_ethernet.h"

char* buf2;
u8* _myip1 = {192, 168, 1, 1};
u8* _netmask1 = {255, 255, 255, 0};
u8* _gatewayip1 = {192, 168, 1, 1};
u8* _dns1 = {8, 8, 8, 8};

void vdatacallback1(u8* buf1, u16 len1) {
	buf2 = (char*) pvPortMalloc(128);
	sprintf(buf2, "You sent %s", buf1);
	Hi_Eth_Send(buf2);
	vPortFree(buf2);
}
void vdhcpcallback1(u8* _ip1) {
}


void vHiInitTask(VP) {
	Hi_Eth_AddCallback(vdatacallback1);
	Hi_Eth_AddDHCPIPCallback(vdhcpcallback1);
	Hi_Eth_Init(_myip1, _netmask1, _gatewayip1, _dns1, 80);
	Hi_LED_Init();
	Hi_IWDG_Enable(1000);
	Hi_LED_SetMode(Hi_LED_Mode_2);
	vTaskDelete(NULL);
}

int main(void) {
	CreateTask2(vHiInitTask);
	vTaskStartScheduler();
	
	while(1) {
	}
}
