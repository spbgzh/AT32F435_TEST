#ifndef __USART_H
#define __USART_H


#include "at32f435_437.h"
#include "stdio.h"
#define USART_RECEVIE_LEN 32
extern uint8_t rxbuf[USART_RECEVIE_LEN];
void usart_print_init(uint32_t baudrate);

struct Receiver_Struct
{
  uint8_t rxbuf[USART_RECEVIE_LEN];
  uint8_t state;
  uint16_t len;
};

extern struct Receiver_Struct Receiver;
#endif
