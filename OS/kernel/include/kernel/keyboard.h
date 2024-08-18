#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <kernel/dt.h>

void keyboard_handler(struct regs* r);
void keyboard_install(void);
void keyboard_capslock();
void keyboard_scrolllock();
void keyboard_numlock();

void keyboard_wait();

#endif