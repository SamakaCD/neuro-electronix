#include "hi_lib.h"
#include "hi_ethernet.h"
#include "stm32f10x.h"
#include "string.h"
#include "Delay.h"
#include "EtherShield.h"
#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "semphr.h"

u8 MYWWWPORT;

static uint8_t buf[BUFFER_SIZE + 1];
static uint16_t plen, dat_p;

static uint8_t* myip;
static uint8_t* mynetmask;
static uint8_t* websrvip;
static uint8_t* gwip;
static uint8_t* dnsip;
static uint8_t* dhcpsvrip;
static uint8_t mymac[6] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x25 };

// New input data callback
static u8 Hi_Eth_BufInitialised = 0;
static void (**Hi_Eth_InputDataCallback) (uint8_t*, uint16_t);
static u8 Hi_Eth_CallbacksCount = 0;

static void (**Hi_Eth_DHCPIPCallbacks) (uint8_t*);
static u8 Hi_Eth_DHCPIPCallbacksCount = 0;

static void Hi_Eth_InitialiseBuffers() {
	if(!Hi_Eth_BufInitialised) {
		Hi_Eth_InputDataCallback = pvPortMalloc(128);
		Hi_Eth_DHCPIPCallbacks   = pvPortMalloc(128);
		Hi_Eth_BufInitialised    = 1;
	}
}

void Hi_Eth_AddCallback(void (*callback) (uint8_t*, uint16_t)) {
	Hi_Eth_InitialiseBuffers();
	Hi_Eth_InputDataCallback[Hi_Eth_CallbacksCount++] = callback;
}

void Hi_Eth_AddDHCPIPCallback(void (*callback) (uint8_t*)) {
	Hi_Eth_InitialiseBuffers();
	Hi_Eth_DHCPIPCallbacks[Hi_Eth_DHCPIPCallbacksCount++] = callback;
}

void Hi_Eth_Loop() {
	dat_p = ES_packetloop_icmp_tcp(buf, ES_enc28j60PacketReceive(BUFFER_SIZE, buf));

	if (dat_p == 0) {
		return;
	}

	for(u8 i = 0; i < Hi_Eth_CallbacksCount; i++) {
		Hi_Eth_InputDataCallback[i](((char *) &(buf[dat_p])), 0);
	}
	memset(buf, 0, BUFFER_SIZE);
}

void Hi_Eth_Send(char* str) {
	u16 len = ES_fill_tcp_data(buf, 0, str);
	www_server_reply(buf, len);
}

void vHi_Eth_LoopTask(VP) {
	while(1) {
		Hi_Eth_Loop();
	}
}

void dhcp_got_ip() {
	for(u8 i = 0; i < Hi_Eth_DHCPIPCallbacksCount; i++)
		Hi_Eth_DHCPIPCallbacks[i](DHCP_IP);
}

static void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;

    /* Reset the RCC clock configuration to default reset state */
    RCC_DeInit();

    /* Configure the High Speed External oscillator */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait for HSE start-up */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Set the code latency value: FLASH Two Latency cycles */
        FLASH_SetLatency(FLASH_Latency_2);

        /* Configure the AHB clock(HCLK): HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* Configure the High Speed APB2 clcok(PCLK2): PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* Configure the Low Speed APB1 clock(PCLK1): PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* Configure the PLL clock source and multiplication factor     */
        /* PLLCLK = HSE*PLLMul = 8*9 = 72MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_3);

        /* Enable PLL   */
        RCC_PLLCmd(ENABLE);

        /* Check whether the specified RCC flag is set or not */
        /* Wait till PLL is ready       */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Get System Clock Source */
        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08);
    }
}

void Hi_Eth_Init(uint8_t* _myip, uint8_t* _netmask, uint8_t*_gwip, uint8_t* _dnsip, uint8_t port) {
	websrvip = myip	 = pvPortMalloc(4);
	mynetmask 		 = pvPortMalloc(4);
	dhcpsvrip = gwip = pvPortMalloc(4);
	dnsip 			 = pvPortMalloc(4);
	MYWWWPORT		 = port;

	memcpy(myip, 	  _myip, 4);
	memcpy(mynetmask, _netmask, 4);
	memcpy(gwip, 	  _gwip, 4);
	memcpy(dnsip, 	  _dnsip, 4);

	taskENTER_CRITICAL(); {
		Hi_Eth_InitialiseBuffers();
		Hi_QGPIO_Out2(GPIOA, GPIO_Pin_8);
		RCC_ClocksTypeDef RCC_Clocks;
		RCC_Configuration();
		RCC_GetClocksFreq(&RCC_Clocks);
		SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 1000);

		ES_enc28j60SpiInit();
		ES_enc28j60Init(mymac);

		bool connectionEstablished = false;
		for(u8 i = 0; i < 10; i++) {
			if(ES_enc28j60Revision() > 0) {
				connectionEstablished = true;
				break;
			}
		}
		if(!connectionEstablished) while(1);

		//set_srv_port(MYWWWPORT);
		if (allocateIPAddress(buf, BUFFER_SIZE, mymac, MYWWWPORT, myip, mynetmask, gwip,
				dhcpsvrip, dnsip) > 0) {

		} else {
			while (1);
		}
	} taskEXIT_CRITICAL();
	CreateTask2(vHi_Eth_LoopTask);
}

void ES_PingCallback() {}
