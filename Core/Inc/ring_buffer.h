/*
 * ring_buffer.h
 *
 *  Created on: Jun 10, 2021
 *      Author: Stek4
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_
#include <stddef.h>
#include <stdint.h>


typedef struct ring_buffer_t ring_buffer_t;
typedef ring_buffer_t * rbuf_handle_t;


void ring_buf_reset(rbuf_handle_t r);
uint8_t ring_buf_empty(rbuf_handle_t r);
uint8_t ring_buf_full(rbuf_handle_t r);
size_t ring_buf_capacity(rbuf_handle_t r);
size_t ring_buf_size(rbuf_handle_t r);
void ring_buf_put(rbuf_handle_t r, uint8_t d);
int ring_buf_put_s(rbuf_handle_t r, uint8_t d);
int ring_buf_get(rbuf_handle_t r, uint8_t * dest);
void init_ring_buf(uint8_t * buffer, rbuf_handle_t rbuf, size_t size);

#endif /* INC_RING_BUFFER_H_ */
