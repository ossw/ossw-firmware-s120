#ifndef COMMAND_OVER_SPI_H
#define COMMAND_OVER_SPI_H

#include <stdbool.h>
#include <stdint.h>

#define SPI_CMD_READ_REG			0x10
#define SPI_CMD_WRITE					0x20

#define SPI_CMD_REG_CMD_INFO	0x0
#define SPI_CMD_REG_CMD_DATA	0x1
#define SPI_CMD_REG_CMD_RESP	0x2

#endif /* COMMAND_OVER_SPI_H */
