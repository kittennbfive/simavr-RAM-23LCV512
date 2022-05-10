#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>

#include "RAM.h"

/*
simavr-RAM-23LCV512

SDI-mode ("dual SPI") not implemented, everything else should work but mostly untested!

Please read the fine manual.

(c) 2022 by kittennbfive

AGPLv3+ and NO WARRANTY!

version 11.05.22 00:45
*/

static uint8_t * ramdata=NULL;

static uint8_t reg_mode=(0<<7)|(1<<6); //default == sequential mode

static uint16_t addr=0;
static uint16_t page=0;
static uint8_t word=0;

typedef enum
{
	MODE_BYTE,
	MODE_PAGE,
	MODE_SEQUENTIAL
} accessmode_t;

static accessmode_t mode=MODE_SEQUENTIAL;

typedef enum
{
	SPI_IDLE,
	SPI_RX_ADDR,
	SPI_READ,
	SPI_WRITE,
	SPI_READ_MODE,
	SPI_WRITE_MODE,
	SPI_INVALID
} state_spi_t;


static state_spi_t state_spi=SPI_IDLE;
static state_spi_t state_spi_next=SPI_IDLE;

////////////////////////////////////////////////////////////////////////

//public functions

void RAM_init(void)
{
	ramdata=malloc(64*1024);
}

void ce_RAM(void * dummy, const uint32_t value) //"dummy" for compatibility with simavr-spi-dispatcher, currently only a single RAM in your project is supported.
{
	(void)dummy;

	if(value==1)
	{
		state_spi=SPI_IDLE;
	}
}

uint8_t spi_RAM(void * dummy, const uint8_t rx) //"dummy" for compatibility with simavr-spi-dispatcher, currently only a single RAM in your project is supported.
{
	(void)dummy;

	uint8_t ret=0xFF;

	static uint8_t addr_bitcount;

	switch(state_spi)
	{
		case SPI_IDLE:
			switch(rx)
			{
				case 0x03:
#ifdef VERBOSE
					printf("RAM: READ\n");
#endif
					addr_bitcount=0;
					state_spi=SPI_RX_ADDR;
					state_spi_next=SPI_READ;
					break;

				case 0x02:
#ifdef VERBOSE
					printf("RAM: WRITE\n");
#endif
					addr_bitcount=0;
					state_spi=SPI_RX_ADDR;
					state_spi_next=SPI_WRITE;
					break;

				case 0x3B:
					errx(1, "RAM: error: unimplemented command 0x3B");
					break;

				case 0xFF:
					errx(1, "RAM: error: unimplemented command 0xFF");
					break;

				case 0x05:
					state_spi=SPI_READ_MODE;
					break;

				case 0x01:
					state_spi=SPI_WRITE_MODE;
					break;

				default:
					errx(1, "RAM: error: unknown command 0x%02X", rx);
					break;
			}
			break;

		case SPI_READ_MODE:
			ret=reg_mode;
			state_spi=SPI_INVALID;
			break;

		case SPI_WRITE_MODE:
			if(rx&~((1<<7)|(1<<6)))
				errx(1, "RAM: error: write mode register: reserved bits must be 0");
			reg_mode=rx;
			switch(reg_mode)
			{
				case ((0<<7)|(0<<6)):
					mode=MODE_BYTE;
					printf("RAM: switched to BYTE mode\n");
					break;

				case ((1<<7)|(0<<6)):
					mode=MODE_PAGE;
					printf("RAM: switched to PAGE mode\n");
					break;

				case ((0<<7)|(1<<6)):
					mode=MODE_SEQUENTIAL;
					printf("RAM: switched to SEQUENTIAL mode\n");
					break;

				default:
					errx(1, "RAM: error: invalid mode specified");
					break;
			}
			state_spi=SPI_INVALID;
			break;

		case SPI_RX_ADDR:
			if(addr_bitcount==0)
			{
				addr=((uint16_t)rx<<8);
				addr_bitcount=8;
			}
			else if(addr_bitcount==8)
			{
				addr|=rx;
				addr_bitcount=16;
				if(mode==MODE_BYTE || mode==MODE_SEQUENTIAL)
#ifdef VERBOSE
					printf("RAM: address set to 0x%04X\n", addr);
#else
					(void)0;
#endif
				else //MODE_PAGE
				{
					page=addr>>5;
					word=addr&0x1F;
#ifdef VERBOSE
					printf("RAM: address set to 0x%04X (page %u word %u)\n", addr, page, word);
#endif
				}
				state_spi=state_spi_next;
			}
			else
				errx(1, "RAM: error: address too long");
			break;

		case SPI_READ:
			switch(mode)
			{
				case MODE_BYTE:
					ret=ramdata[addr];
					state_spi=SPI_INVALID;
					break;

				case MODE_PAGE:
					ret=ramdata[32*page+word++];
					if(word==32)
						word=0;
					break;

				case MODE_SEQUENTIAL:
					ret=ramdata[addr++];
					break;
			}
			break;

		case SPI_WRITE:
			switch(mode)
			{
				case MODE_BYTE:
					ramdata[addr]=rx;
					state_spi=SPI_INVALID;
					break;

				case MODE_PAGE:
					ramdata[32*page+word++]=rx;
					if(word==32)
						word=0;
					break;

				case MODE_SEQUENTIAL:
					ramdata[addr++]=rx;
					break;
			}
			break;

		case SPI_INVALID:
			errx(1, "RAM: error: invalid spi state");
			break;
	}

	return ret;
}

void RAM_cleanup(void)
{
	free(ramdata);
}
