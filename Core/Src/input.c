/*
 * input.c
 *
 *  Created on: Jun 8, 2021
 *      Author: Stek4
 */


#include <stdint.h>
#include <stdio.h>
#include "input.h"

void UART_ReceiveEnable(void);
void init_ring_buf(void);


typedef enum {
  idle_input,
  token_reveive,
  backspace
}input_flags_t;



uint8_t UARTDataRX = 0;
volatile input_flags_t flags;
volatile uint8_t * g_input;

UART_HandleTypeDef * uart;


void init_input(UART_HandleTypeDef * huart)
{
  uart = huart;
  init_ring_buf();
  UART_ReceiveEnable();
}

//Start receive next byte.
void UART_ReceiveEnable(void)
{
  HAL_UART_Receive_IT(uart, (uint8_t*)&UARTDataRX, 1);
}






typedef struct ring_buffer_t ring_buffer_t;
typedef ring_buffer_t * rbuf_handle_t;

typedef struct ring_buffer_t
{
  uint8_t * buffer;
  size_t head;
  size_t tail;
  size_t max;
  uint8_t full;
}ring_buffer_t;




#define INPUT_BUFFER_SIZE 128
uint8_t buffer[2][INPUT_BUFFER_SIZE] = {{0},{0}};

//rbuf_handle_t rbuf[2];

rbuf_handle_t rbuf;

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

void init_ring_buf(void)
{

  static uint8_t rbuf_handle_pool[2][sizeof(ring_buffer_t) + 1] = {{0},{0}};
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
  rbuf = (rbuf_handle_t)(&rbuf_handle_pool[0][0]);
  rbuf->buffer = &buffer[0][0];
  rbuf->max = INPUT_BUFFER_SIZE;

  ring_buf_reset(rbuf);
//  assert(ring_buf_empty(rbuf));

}


static void insert_char_handle(uint8_t ch)
{

  // в этот момент мы считаем, что полученный символ должен быть записан в строку входящего сообщения,
  // однако, нам нужна проверка на границу массива. для этого пригодился бы кольцевой буффер.
  ring_buf_put_s(rbuf, ch);
}


static void backspace_handle(void)
{
  if(!ring_buf_empty(rbuf))
  {
    retreat_pointer(rbuf);
  }

}

static void eol_handle(void)
{
//switch input buffer pointer
  uint8_t  data;
  printf("get message:\r\n");

  while(!ring_buf_get(rbuf, &data))
  {
    printf("%c", data);
  }

  printf("\r\n");
}

static void tab_handle(void)
{

}

void insert_character(uint8_t ch)
{
  switch(ch)
  {
    case '\b': //backspace
      backspace_handle();
      break;
    case '\r': //carriage return
    case '\n': //line feed
      eol_handle();
      break;
    case '\t': // tab
      tab_handle();
      break;
    default:
      insert_char_handle(ch);
      break;
  }
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART6)
  {
#if ECHO_ON
      HAL_UART_Transmit(huart, &UARTDataRX, 1, 100); //echo
#endif

    insert_character(UARTDataRX);

    UART_ReceiveEnable();
  }
}

