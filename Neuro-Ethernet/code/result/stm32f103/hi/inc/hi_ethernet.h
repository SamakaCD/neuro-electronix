#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#define BUFFER_SIZE 550

void Hi_Eth_AddCallback(void (*callback) (uint8_t*, uint16_t));
void Hi_Eth_AddDHCPIPCallback(void (*callback) (uint8_t*));
void Hi_Eth_Init(uint8_t* _myip, uint8_t* _netmask, uint8_t* _gwip, uint8_t* _dnsip, uint8_t port);
void Hi_Eth_Send(char* str);
