#ifndef _HELPER_H_
#define _HELPER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

// operations to set, clear, toggle, and check individual register bits
#define set(reg,bit)	  reg |= (1<<(bit))
#define clear(reg,bit)	  reg &= ~(1<<(bit))
#define toggle(reg,bit)	  reg ^= (1<<(bit))
#define check(reg,bit)	  (reg & (1<<(bit)))

#endif
