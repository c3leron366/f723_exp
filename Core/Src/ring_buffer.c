/*
 * ring_buffer.c
 *
 *  Created on: Jun 10, 2021
 *      Author: Stek4
 */
#include "ring_buffer.h"



typedef struct ring_buffer_t
{
  uint8_t * buffer;
  size_t head;
  size_t tail;
  size_t max;
  uint8_t full;
}ring_buffer_t;


void ring_buf_reset(rbuf_handle_t r)
{
//  assert(r);
  r->head = 0;
  r->tail = 0;
  r->full = 0;
}

uint8_t ring_buf_empty(rbuf_handle_t r)
{
//  assert(r);
  return (uint8_t)(!r->full && (r->head == r->tail));
}

uint8_t ring_buf_full(rbuf_handle_t r)
{
//  assert(r);
  return (uint8_t)(r->full);
}

size_t ring_buf_capacity(rbuf_handle_t r)
{
//  assert(r);
  return (r->max);
}

size_t ring_buf_size(rbuf_handle_t r)
{
//  assert(r);

  size_t size = r->max;

  if(!r->full)
  {
    if(r->head >= r->tail)
    {
      size = r->head - r->tail;
    }
    else
    {
      size = (r->max + r->head) - r->tail;
    }
  }

  return size;
}

static void advance_pointer(rbuf_handle_t r)
{
  //assert(r);
  if(r->full)
  {
    if(++(r->tail) == r->max)
    {
      r->tail = 0;
    }
  }

  if(++(r->head) == r->max)
  {
    r->head = 0;
  }
  r->full = r->head == r->tail;
}

static void retreat_pointer(rbuf_handle_t r)
{
  //assert(r);

  r->full = 0;
  if(++(r->tail) == r->max)
  {
    r->tail = 0;
  }
}

void ring_buf_put(rbuf_handle_t r, uint8_t d)
{
//  assert(r && r->buffer);
  r->buffer[r->head] = d;
  advance_pointer(r);
}

int ring_buf_put_s(rbuf_handle_t r, uint8_t d)
{
  int result = -1;
//  assert(r && r->buffer);
  if(!ring_buf_full(r))
  {
     r->buffer[r->head] = d;
     advance_pointer(r);
     r = 0;
  }
  return result;
}

int ring_buf_remove_last_symbol(rbuf_handle_t r)
{
  int result = -1;

  if(!ring_buf_empty(r))
  {
     r->buffer[r->head] = 0;
     r->head -= 1;
     r->full = 0;
     result = 0;
  }


  return result;
}

int ring_buf_get(rbuf_handle_t r, uint8_t * dest)
{
//  assert(r && dest && r->buffer);
  int result = -1;

  if(!ring_buf_empty(r))
  {
    *dest = r->buffer[r->tail];
    retreat_pointer(r);
    result = 0;
  }
  return result;
}



void init_ring_buf(uint8_t * buffer, rbuf_handle_t rbuf, size_t size)
{

//  static uint8_t rbuf_handle_pool[2][sizeof(ring_buffer_t) + 1] = {{0},{0}};
/*
  rbuf[0] = (rbuf_handle_t)(&rbuf_handle_pool[0][0]);
  rbuf[0]->buffer = &buffer[0][0];
  rbuf[0]->head = 0;
  rbuf[0]->tail = 0;
  rbuf[0]->max = INPUT_BUFFER_SIZE;
  rbuf[0]->full = 0;

  rbuf[1] = (rbuf_handle_t)(&rbuf_handle_pool[1][0]);
  rbuf[1]->buffer = &buffer[1][0];
  rbuf[1]->head = 0;
  rbuf[1]->tail = 0;
  rbuf[1]->max = INPUT_BUFFER_SIZE;
  rbuf[1]->full = 0;
*/
//  rbuf = (rbuf_handle_t)(&rbuf_handle_pool[0][0]);
  rbuf->buffer = buffer;
  rbuf->max = size;

  ring_buf_reset(rbuf);
//  assert(ring_buf_empty(rbuf));

}
