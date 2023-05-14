#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Header includes -----------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "malloc.h" 

/* Macro definitions ---------------------------------------------------------*/
#define RING_BUFFER_MALLOC(size)  mymalloc(SRAMIN,size)
#define RING_BUFFER_FREE(block)   myfree(SRAMIN,block)

/* Type definitions ----------------------------------------------------------*/
typedef struct
{
  uint8_t *buffer;
  uint32_t size;
  uint32_t in;
  uint32_t out;
}RingBuffer;

/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
RingBuffer *RingBuffer_Malloc(uint32_t size);
void RingBuffer_Free(RingBuffer *fifo);

uint32_t RingBuffer_In(RingBuffer *fifo, void *in, uint32_t len);
uint32_t RingBuffer_Out(RingBuffer *fifo, void *out, uint32_t len);

/* Function definitions ------------------------------------------------------*/

/**
  * @brief  Removes the entire FIFO contents.
  * @param  [in] fifo: The fifo to be emptied.
  * @return None.
  */
static inline void RingBuffer_Reset(RingBuffer *fifo)
{
  fifo->in = fifo->out = 0;
}

/**
  * @brief  Returns the size of the FIFO in bytes.
  * @param  [in] fifo: The fifo to be used.
  * @return The size of the FIFO.
  */
static inline uint32_t RingBuffer_Size(RingBuffer *fifo)
{
  return fifo->size;
}

/**
  * @brief  Returns the number of used bytes in the FIFO.
  * @param  [in] fifo: The fifo to be used.
  * @return The number of used bytes.
  */
static inline uint32_t RingBuffer_Len(RingBuffer *fifo)
{
  return fifo->in - fifo->out;
}

/**
  * @brief  Returns the number of bytes available in the FIFO.
  * @param  [in] fifo: The fifo to be used.
  * @return The number of bytes available.
  */
static inline uint32_t RingBuffer_Avail(RingBuffer *fifo)
{
  return RingBuffer_Size(fifo) - RingBuffer_Len(fifo);
}

/**
  * @brief  Is the FIFO empty?
  * @param  [in] fifo: The fifo to be used.
  * @retval true:      Yes.
  * @retval false:     No.
  */
static inline bool RingBuffer_IsEmpty(RingBuffer *fifo)
{
  return RingBuffer_Len(fifo) == 0;
}

/**
  * @brief  Is the FIFO full?
  * @param  [in] fifo: The fifo to be used.
  * @retval true:      Yes.
  * @retval false:     No.
  */
static inline bool RingBuffer_IsFull(RingBuffer *fifo)
{
  return RingBuffer_Avail(fifo) == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __RINGBUFFER_H */
