#ifndef _RS232_H
#define _RS232_H

int rs232_port_decider(int port);
int rs232_init(int port, long baudrate);
int rs232_is_transmit_empty(int port);
int rs232_write(int port, char character);
void irq_rs232_port1_handler(void*);
void irq_rs232_port2_handler(void*);

#endif