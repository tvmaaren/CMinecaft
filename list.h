typedef struct{
	void *l;
	size_t available;
	size_t used;
}List;


List list_init();
List list_init_with(size_t s,void* l);
void list_free(List l);
void list_append(List *l,size_t s);
void list_concat(List *l,size_t s, void* p);
void list_pop(List *l,size_t offset,size_t s);
