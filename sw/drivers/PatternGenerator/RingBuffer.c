
#include "../common/common.h"
#include "RingBuffer.h"
#include <linux/circ_buf.h>
#include <linux/string.h>
#include <asm/barrier.h>
#include <asm/compiler.h>



bool ringBufferIsEmpty(RingBuffer* ringBuffer)
{
    return (CIRC_SPACE(ringBuffer->head, ringBuffer->tail, RINGBUFFER_SIZE) == (RINGBUFFER_SIZE-1U));
}

bool ringBufferIsFull(RingBuffer* ringBuffer)
{
    return (CIRC_SPACE(ringBuffer->head, ringBuffer->tail, RINGBUFFER_SIZE) == 0U);
}

void ringBufferPut(RingBuffer* ringBuffer, uint32_t word)
{
        unsigned long head = ringBuffer->head;
        unsigned long tail = ringBuffer->tail;

        if (CIRC_SPACE(head, tail, RINGBUFFER_SIZE) >= 1) {
            ringBuffer->buffer[head] = word;

            smp_store_release(&ringBuffer->head,
                      (head + 1) & (RINGBUFFER_SIZE - 1));
        }
}


uint32_t ringBufferGet(RingBuffer* ringBuffer)
{
    uint32_t word = 0U;

    unsigned long head = smp_load_acquire(&ringBuffer->head);
    unsigned long tail = ringBuffer->tail;

    if (CIRC_CNT(head, tail, RINGBUFFER_SIZE) >= 1) {

            word = ringBuffer->buffer[tail];

            smp_store_release(&ringBuffer->tail,
                            (tail + 1) & (RINGBUFFER_SIZE - 1));
    }

    return word;
}

void ringBufferClear(RingBuffer* ringBuffer)
{
    memset(ringBuffer->buffer, 0, RINGBUFFER_SIZE * sizeof(uint32_t));
    ringBuffer->head = 0U;
    ringBuffer->tail = 0U;
}

void ringBufferInit(RingBuffer* ringBuffer)
{
    ringBufferClear(ringBuffer);
}