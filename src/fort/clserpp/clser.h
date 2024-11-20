// SPDX-License-Identifier: None This file has no license, just interpolated
// from some stuff found arround, include cameralink specifications and some
// clsermv.h file with no license found.
#pragma once

#include <cstdint>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *clSerialRef_t;

typedef enum {
	CL_BAUDRATE_9600   = 1,
	CL_BAUDRATE_19200  = 2,
	CL_BAUDRATE_38400  = 4,
	CL_BAUDRATE_57600  = 8,
	CL_BAUDRATE_115200 = 16,
	CL_BAUDRATE_230400 = 32,
	CL_BAUDRATE_460800 = 64,
	CL_BAUDRATE_921600 = 128,

} clBaudrate_e;

typedef enum {
	CL_VERSION_NONE = 1,
	CL_VERSION_1_0  = 2,
	CL_VERSION_1_1  = 3
} clVersion_e;

int32_t clSerialInit(uint32_t index, clSerialRef_t *ref);
void    clSerialClose(clSerialRef_t serial);
int32_t clSerialRead(
    clSerialRef_t serial, char *buffer, uint32_t *size, uint32_t timeout_ms
);

int32_t clSerialWrite(
    clSerialRef_t serial, char *buffer, uint32_t *size, uint32_t timeout_ms
);

// this is not conventional but work in my use case mvHyperion

int32_t clFlushPort(clSerialRef_t serial);
int32_t clGetErrorText(int32_t errorCode, char *buffer, uint32_t *size);
int32_t clGetManufacturerInfo(char *buffer, uint32_t *size, uint32_t *version);
int32_t clGetNumBytesAvail(clSerialRef_t serial, uint32_t *numBytes);

int32_t clGetNumSerialPorts(uint32_t *serialPorts);
int32_t clGetSerialPortIdentifier(uint32_t idx, char *buffer, uint32_t *size);
int32_t clGetSupportedBaudRates(clSerialRef_t serial, uint32_t *baudrates);

#ifdef __cplusplus
}
#endif
