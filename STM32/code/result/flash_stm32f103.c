#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)

uint8_t Hi_Flash_IsReady(void) {
	return !(FLASH->SR & FLASH_SR_BSY);
}

void Hi_Flash_EraseAllPages(void) {
    FLASH->CR |= FLASH_CR_MER;
    FLASH->CR |= FLASH_CR_STRT;
    while(!Hi_Flash_IsReady()) {}
    FLASH->CR &= FLASH_CR_MER;
}

void Hi_Flash_ErasePage(uint32_t address) {
    FLASH->CR|= FLASH_CR_PER;
    FLASH->AR = address;
    FLASH->CR|= FLASH_CR_STRT;
    while(!Hi_Flash_IsReady()) {}
    FLASH->CR&= ~FLASH_CR_PER;
}

void Hi_Flash_Unlock(void) {
	  FLASH->KEYR = FLASH_KEY1;
	  FLASH->KEYR = FLASH_KEY2;
}

void Hi_Flash_Lock() {
	FLASH->CR |= FLASH_CR_LOCK;
}

#define FlashRdy() while(!Hi_Flash_IsReady()) {}

uint32_t Hi_Flash_Read32(uint32_t address) {
	return (*(__IO uint32_t*) address);
}

uint16_t Hi_Flash_Read16(uint32_t address) {
	return (*(__IO uint16_t*) address);
}

uint8_t Hi_Flash_Read1(uint32_t address) {
	return (uint8_t) (Hi_Flash_Read16(address) & 0xFF);
}

uint8_t Hi_Flash_Read2(uint32_t address) {
	return (uint8_t) ((Hi_Flash_Read16(address) >> 8) & 0xFF);
}

uint32_t Hi_Flash_GetPage(uint8_t pageNumber) {
	return FLASH_BASE + 2048 * pageNumber;
}

uint32_t Hi_Flash_GetOPage(uint8_t pageNumber, uint16_t offset) {
	return FLASH_BASE + 2048 * pageNumber + offset;
}

void Hi_Flash_StartRecord() {
	FLASH->CR |= FLASH_CR_PG;
	FlashRdy();
}

void Hi_Flash_EndRecord() {
	FLASH->CR &= ~(FLASH_CR_PG);
}

void Hi_Flash_Write(uint32_t address, uint8_t b1, uint8_t b2) {
	if(address % 2 == 0) {
		*(__IO uint16_t*) address = ((b2 & 0xFF) << 8) | (b1 & 0xFF);
	} else {
		Hi_Flash_Write(address-1, 0xFF, b1);
		Hi_Flash_Write(address+1, b2, 0xFF);
	}
    while(FLASH->SR & FLASH_SR_BSY) {}
}

void Hi_Flash_Write16(uint32_t address, uint16_t data) {
    *(__IO uint16_t*) address = data;
    while(FLASH->SR & FLASH_SR_BSY) {}
}
