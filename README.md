# simavr-RAM-23LCV512

## What is this?
This code simulates a 64kB SPI-RAM (23LCV512) for use with [simavr](https://github.com/buserror/simavr).

## Licence and disclaimer
AGPLv3+ and NO WARRANTY!

## Limitations
SDI-mode ("dual-SPI") is unsupported and there can be only a single RAM inside your project.

## Compile-time configuration
You can uncomment `#define VERBOSE` in the header-file to make the code print details about what is happening.

## public API
```
void RAM_init(void);
void ce_RAM(void * dummy, const uint32_t value);
uint8_t spi_RAM(void * dummy, const uint8_t rx);
void RAM_cleanup(void);
```

### RAM_init
This function must be called before any other function to set some internal things up.

### ce_RAM and spi_RAM
Those are the callbacks you need to provide to the SPI-dispatcher, see documentation there. It should be possible to use this code without the SPI-dispatcher but you might need to write some glue-code. Beware that there can only be one RAM inside your project (Use `NULL` for the pointer to the internal data structure you specify when initializing the SPI-dispatcher).

### RAM_cleanup
To be called once the simulation has finished to clean up some internal stuff.
