
#include <linux/types.h>
#include <linux/wait.h>

#define RINGBUFFER_SIZE 512U

typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t buffer[RINGBUFFER_SIZE];
    struct wait_queue_head consumer;
} RingBuffer;

bool ringBufferIsEmpty(RingBuffer* ringBuffer);
void ringBufferPut(RingBuffer* ringBuffer, uint32_t word);
uint32_t ringBufferGet(RingBuffer* ringBuffer);
void ringBufferClear(RingBuffer* ringBuffer);
void ringBufferInit(RingBuffer* ringBuffer);