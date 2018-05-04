#ifndef RINGBUFF_H
#define RINGBUFF_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stddef.h"

typedef struct{
	short* buffer;
	size_t head;
	size_t tail;
	size_t size;
	size_t elsize;
}RingBuffer;

int rb_init(RingBuffer* r, short* buff, size_t size);

void rb_destroy(RingBuffer* r);

size_t rb_push(RingBuffer* r, short* data_in, size_t num);

size_t rb_pop(RingBuffer* r, short* data_out, size_t num);

size_t rb_pushAvail(RingBuffer* r); 

size_t rb_popAvail(RingBuffer* r);

#ifdef __cplusplus
}
#endif

#endif /* RINGBUFF_H */
