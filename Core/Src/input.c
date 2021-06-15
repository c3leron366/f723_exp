/*
 * input.c
 *
 *  Created on: Jun 8, 2021
 *      Author: Stek4
 */


#include <stdint.h>
#include <stdio.h>
#include "input.h"
#include "ring_buffer.h"



void UART_ReceiveEnable(void);

typedef enum {
  idle_input,
  token_reveive,
  backspace
}input_flags_t;



uint8_t UARTDataRX = 0;
volatile input_flags_t flags;
volatile uint8_t * g_input;

UART_HandleTypeDef * uart;

#define INPUT_BUFFER_SIZE 128
uint8_t buffer[2][INPUT_BUFFER_SIZE] = {{0},{0}};
rbuf_handle_t _rbuf[2];
rbuf_handle_t rbuf;
//TODO refactor ring buffer init function!
uint8_t rbuf_handle_pool[2][sizeof(ring_buffer_t) + 1] = {{0},{0}};
volatile uint8_t rbuf_idx = 0;

void init_input(UART_HandleTypeDef * huart)
{
  uart = huart;

  _rbuf[0] = (rbuf_handle_t)(&rbuf_handle_pool[0][0]);
  _rbuf[1] = (rbuf_handle_t)(&rbuf_handle_pool[1][0]);

  init_ring_buf(
		(uint8_t *)&buffer[0][0],
		_rbuf[0],
		INPUT_BUFFER_SIZE);

  init_ring_buf(
		(uint8_t *)&buffer[1][0],
		_rbuf[1],
		INPUT_BUFFER_SIZE);
  rbuf = _rbuf[rbuf_idx];
  UART_ReceiveEnable();
}

//Start receive next byte.
void UART_ReceiveEnable(void)
{
  HAL_UART_Receive_IT(uart, (uint8_t*)&UARTDataRX, 1);
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
      ring_buf_remove_last_symbol(rbuf);
  }

}
volatile uint8_t recv_flag = 0;
static void eol_handle(void)
{
  //в таком виде будет проблема при приеме двух терминальных символов подряд
//switch input buffer pointer
//  rbuf_idx ^= 1;
  rbuf = _rbuf[rbuf_idx];
//and raise got_message flag
  recv_flag += 1;
}

void proc_input(void)
{

  if(recv_flag)
  {
    uint8_t data;
    uint8_t tx_idx;


    tx_idx = rbuf_idx;
//    tx_idx = rbuf_idx^1;
    printf("buf %d: ", tx_idx );

    while(!ring_buf_get(_rbuf[tx_idx], &data))
    {
      printf("%c", data);
    }

    printf("\r\n");

    if(recv_flag != 0)
      recv_flag -= 1;

  }
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

