/************************************************************************/
/* \file spi.h                                                          */
/************************************************************************/
#ifndef SPI_H_INCLUDED
#define SPI_H_INCLUDED

#define MAX_SPI_READ_LENGTH 3
void spi_init(void);
void spi_enable(bool);
void poll_spi(void);

#endif
