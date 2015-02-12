#define Hi_List_StartMem 32

typedef struct {
	void** 		List;
	uint32_t	Count;
	uint32_t	Mem;
} Hi_List;

void Hi_List_Create(Hi_List* list) {
	void** lstBuf = (void**) pvPortMalloc(Hi_List_StartMem);
	list -> List    = lstBuf;
	list -> Count   = 0;
	list -> Mem     = Hi_List_StartMem;
}

void Hi_List_Add(Hi_List* list, void* item) {
	if((list->Count) >= (list->Mem)) {
		u32 i;
		(list->Mem) *= 2;
		void** newLst = (void**) pvPortMalloc(list->Mem);
		for(i = 0; i < (list->Mem)/2; i++) {
			newLst[i] = (list->List)[i];
		}
		vPortFree(list->List);
		list -> List = newLst;
		newLst[(list->Count)++] = item;
	} else {
		(list->List)[(list->Count)++] = item;
	}
}

void* Hi_List_Get(Hi_List* list, uint32_t index) {
	return (list->List)[index];
}
