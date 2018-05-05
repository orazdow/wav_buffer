#include "ringbuffer.h"
#include "stdlib.h"

size_t rb_push(RingBuffer* r, short* data, size_t num){
	size_t written = 0; 	
	while((r->head+1) % r->size != r->tail && written != num){
		r->buffer[r->head] = *data++;
		r->head = (r->head+1) % r->size; 
		written++;
	}
	return written;
}

size_t rb_pop(RingBuffer* r, short* data, size_t num){
	size_t written = 0; 
	while(r->tail != r->head && written != num){ 

		*data++ = r->buffer[r->tail];
		r->tail = (r->tail+1) % r->size;
		written++;

	}
	return written;
}

size_t rb_pushAvail(RingBuffer* r){
	
	if(r->head > r->tail){
		return (r->size - r->head) + r->tail;

	}else if(r->head < r->tail){
		return r->tail - r->head;

	}else{
		return r->size;
	}
	
}

size_t rb_popAvail(RingBuffer* r){
	
	if(r->head > r->tail){
		return r->head - r->tail;

	}else if(r->head < r->tail){
		return r->head + (r->size - r->tail);

	}else{
		return 0;
	}

}

int rb_init(RingBuffer* r, short* buff, size_t size){
	r->size = size;
	if(buff == NULL){
		r->buffer = (short*)malloc(size*sizeof(short));
	}else{
		r->buffer = (short*)buff;
	}
	r->head = 0;
	r->tail = 0;
	if(r->buffer == NULL)
		return -1;

	return 0;
}

void rb_destroy(RingBuffer* r){
	free(r->buffer);
}
