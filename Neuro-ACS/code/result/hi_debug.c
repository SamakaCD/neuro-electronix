#include "stm32f10x.h"

u32 Hi_Debug_Cnt = 0;

void Hi_Debug_Init() {
	if(!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}
}

void Hi_Debug_StartTimer() {
	Hi_Debug_Cnt = DWT -> CYCCNT;
}

u32 Hi_Debug_GetMS() {
	return ((DWT -> CYCCNT) - Hi_Debug_Cnt)/72000;
}

u32 Hi_Debug_GetUS() {
	return ((DWT -> CYCCNT) - Hi_Debug_Cnt)/72;
}
