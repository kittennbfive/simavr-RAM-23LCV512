#ifndef __RAM_H__
#define __RAM_H__
#include <stdint.h>

#include "sim_avr.h"
#include "sim_irq.h"

/*
public header for simavr-RAM-23LCV512

(c) 2022 by kittennbfive

AGPLv3+ and NO WARRANTY!

version 10.05.22 22:30
*/

//#define VERBOSE //show details

void RAM_init(void);
void ce_RAM(void * dummy, const uint32_t value);
uint8_t spi_RAM(void * dummy, const uint8_t rx);
void RAM_cleanup(void);

#endif
