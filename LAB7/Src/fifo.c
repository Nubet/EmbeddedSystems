#include "fifo.h"

static uint8_t FIFO_NextIndex(uint8_t index) {
	return (uint8_t) ((index + 1U) & FIFO_MASK);
}

void FIFO_Init(FIFO *fifo) {
	fifo->head = 0;
	fifo->tail = 0;
}

bool FIFO_IsEmpty(FIFO *fifo) {
	return fifo->head == fifo->tail;
}

bool FIFO_IsFull(FIFO *fifo) {
	// reaching the tail means no more empty cells left
	return FIFO_NextIndex(fifo->head) == fifo->tail;
}

FifoStatus FIFO_Put(FIFO *fifo, char data) {
	if (FIFO_IsFull(fifo)) {
		return FIFO_ERROR_FULL;
	}
	fifo->buffer[fifo->head] = data;
	fifo->head = FIFO_NextIndex(fifo->head);
	return FIFO_OK;
}

FifoStatus FIFO_Get(FIFO *fifo, char *data) {
	if (FIFO_IsEmpty(fifo)) {
		return FIFO_ERROR_EMPTY;
	}
	*data = fifo->buffer[fifo->tail];
	fifo->tail = FIFO_NextIndex(fifo->tail);
	return FIFO_OK;
}
