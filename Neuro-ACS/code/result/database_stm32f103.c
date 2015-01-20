char** Hi_DB_Keys;
int*   Hi_DB_ID;

void Hi_DB_Init(char** db_keys, int* ids) {
	Hi_DB_Keys = db_keys;
	Hi_DB_ID = ids;
}

int Hi_DB_Read(char* key) {
	int i;
	for(i = 0; i < DB_CNT; i++) {
		if(strstr(Hi_DB_Keys[i], key)) {
			return Hi_DB_ID[i];
		}
	}

	return -1;
}
