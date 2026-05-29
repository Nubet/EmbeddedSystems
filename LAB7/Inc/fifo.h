#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>

#define FIFO_SIZE 8
#define FIFO_MASK (FIFO_SIZE - 1)

typedef struct FIFO {
    volatile char buffer[FIFO_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} FIFO;

typedef enum {
    FIFO_OK = 0,
    FIFO_ERROR_FULL,
    FIFO_ERROR_EMPTY
} FifoStatus;

void FIFO_Init(FIFO *fifo);
bool FIFO_IsEmpty(FIFO *fifo);
bool FIFO_IsFull(FIFO *fifo);
FifoStatus FIFO_Put(FIFO *fifo, char data);
FifoStatus FIFO_Get(FIFO *fifo, char *data);

#endif // FIFO_H
