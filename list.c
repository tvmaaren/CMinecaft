#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

List list_init(size_t size){
	List ret;
	ret.l = malloc(2*size*10);
	ret.available = 2*size+10;
	ret.used =size;
	return ret;
}
List list_init_with(size_t s,void* l){
	List ret;
	ret.l = malloc(2*s);
	ret.available = 2*s;
	ret.used =s;
	memcpy(ret.l,l,s);
	return ret;
}

void list_free(List l){
	free(l.l);
}

void list_append(List *l,size_t s){
	l->used+=s;
	_Bool resize=false;
	while(l->used>l->available){
		resize=true;
		l->available*=2;
		if(l->available==0)l->available+=2;
	}
	if(resize)l->l = realloc(l->l,l->available);
}

void list_pop(List *l,size_t offset,size_t s){
	for(int i=offset+s;i<l->used;i++){
		((char*)(l->l))[i-s]=((char*)(l->l))[i];
	}
	l->used-=s;
}

void list_concat(List *l,size_t s, void* p){
	int used_prev = l->used;
	list_append(l,s);
	memcpy(l->l+used_prev,p,s);
}
// vim: cc=100 
