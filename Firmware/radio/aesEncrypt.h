// -*- Mode: C; c-basic-offset: 8; -*-
//
// Copyright (c) 2013 Luke Hovington, All Rights Reserved
// Copyright 2011 Silicon Laboratories, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  o Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  o Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//

///
/// @file	aesEncrypt.h
///
/// Prototypes for the AT command parser
///

#ifndef _AES_ENCRYPT_H_
#define _AES_ENCRYPT_H_
#include "board.h"
#include <stdint.h>

#ifdef CPU_SI1030 // Only CPU_SI1030 has encription

#define DMA_TRANSFERS_USE_IDLE
#define INCLUDE_ENCRYPTION

#include "radio.h"

//-----------------------------------------------------------------------------
// typedefs for key size and status
//-----------------------------------------------------------------------------
typedef U8 GENERATE_DECRYPTION_KEY_SIZE;
typedef U8 GENERATE_DECRYPTION_KEY_STATUS;

//=============================================================================
// Function Prototypes (API)
//=============================================================================

void aesEncrypt_init();

uint8_t aesEncryptDecrypt(__pdata uint8_t operation,
						  VARIABLE_SEGMENT_POINTER(plainText, uint8_t, SEG_XDATA),
						  VARIABLE_SEGMENT_POINTER(cipherText, uint8_t, SEG_XDATA),
						  __pdata uint16_t blocks);

//-----------------------------------------------------------------------------
// enum used for ENCRYPT_DECRYPT_AND_SIZE
//-----------------------------------------------------------------------------
enum ENCRYPT_DECRYPT_AND_SIZE_Enum
{
	DECRYPTION_128_BITS = 0,            // 0x00
	DECRYPTION_192_BITS,                // 0x01
	DECRYPTION_256_BITS,                // 0x02
	DECRYPTION_UNDEFINED,               // 0x03
	ENCRYPTION_128_BITS,                // 0x04
	ENCRYPTION_192_BITS,                // 0x05
	ENCRYPTION_256_BITS,                // 0x06
	ENCRYPTION_UNDEFINED                // 0x07
};

enum ENCRYPT_DECRYPT_Enum
{
	DECRYPTION = 0,
	ENCRYPTION
};

//-----------------------------------------------------------------------------
// enum used for KEY_SIZE
//-----------------------------------------------------------------------------
enum KEY_SIZE_Enum
{
	KEY_SIZE_128_BITS = 0,             // 0x00
	KEY_SIZE_192_BITS,                 // 0x01
	KEY_SIZE_256_BITS,                 // 0x02
	KEY_SIZE_UNDEFINED                 // 0x03
};

//-----------------------------------------------------------------------------
// DMA transfers use Idle mode
//-----------------------------------------------------------------------------
#define DMA_TRANSFERS_USE_IDLE
//=============================================================================
// Static DMA Channel Allocations (Static)
//
// These defines are used for a Static DMA allocation. The DMA channels are
// assigned for a specific purpose.
//
// These settings reuse the AES DMA channels for the encoder/decoder.
// So these operations cannot be done simultaneously.
//
//=============================================================================
#define  SPI1_IN_CHANNEL   0x0
#define  SPI1_OUT_CHANNEL  0x1
#define  CRC1_IN_CHANNEL   0x2
#define  ENC0_IN_CHANNEL   0x3
#define  ENC0_OUT_CHANNEL  0x4
#define  AES0KIN_CHANNEL   0x3
#define  AES0BIN_CHANNEL   0x4
#define  AES0XIN_CHANNEL   0x5
#define  AES0YOUT_CHANNEL  0x6

//-----------------------------------------------------------------------------
// DMA Bits
//
// Enable/Disable and Interrupt bits based on above static allocations.
//
//-----------------------------------------------------------------------------
#define  ENC0_IN_MASK      (1<<ENC0_IN_CHANNEL)
#define  ENC0_OUT_MASK     (1<<ENC0_OUT_CHANNEL)
#define  ENC0_MASK         (ENC0_IN_MASK|ENC0_OUT_MASK)
#define  CRC1_IN_MASK      (1<<CRC1_IN_CHANNEL)
#define  SPI1_IN_MASK      (1<<SPI1_IN_CHANNEL)
#define  SPI1_OUT_MASK     (1<<SPI1_OUT_CHANNEL)
#define  SPI1_MASK         (SPI1_IN_MASK|SPI1_OUT_MASK)
#define  AES0KIN_MASK      (1<<AES0KIN_CHANNEL)
#define  AES0BIN_MASK      (1<<AES0BIN_CHANNEL)
#define  AES0XIN_MASK      (1<<AES0XIN_CHANNEL)
#define  AES0YOUT_MASK     (1<<AES0YOUT_CHANNEL)
#define  AES0_KBXY_MASK    (AES0KIN_MASK|AES0BIN_MASK|AES0XIN_MASK|AES0YOUT_MASK)
#define  AES0_KBY_MASK     (AES0KIN_MASK|AES0BIN_MASK|AES0YOUT_MASK)

//=============================================================================
// DMA Peripheral Requests
//
// IN/OUT defined from the peripheral's perspective.
//
// IN    =  XRAM -> SFR
// OUT   =  SFR -> XRAM
//
// SPI1 Master mode
// SPI1_IN  =  XRAM -> SFR = SPI Write = MOSI data
// SPI1_OUT =  SFR -> XRAM = SPI Read  = MISO data
//
// SPI1 Slave mode
// SPI1_IN  =  XRAM -> SFR = SPI Write = MISO data
// SPI1_OUT =  SFR -> XRAM = SPI Read  = MOSI data
//
//-----------------------------------------------------------------------------
enum PERIPHERAL_REQUEST_Enum
{
	ENC0_IN_PERIPHERAL_REQUEST = 0,     // 0x0
	ENC0_OUT_PERIPHERAL_REQUEST,        // 0x1
	CRC1_PERIPHERAL_REQUEST,            // 0x2
	SPI1_IN_PERIPHERAL_REQUEST,         // 0x3
	SPI1_OUT_PERIPHERAL_REQUEST,        // 0x4
	AES0KIN_PERIPHERAL_REQUEST,         // 0x5
	AES0BIN_PERIPHERAL_REQUEST,         // 0x6
	AES0XIN_PERIPHERAL_REQUEST,         // 0x7
	AES0YOUT_PERIPHERAL_REQUEST         // 0x8
};

//-----------------------------------------------------------------------------
// defines used with DMA0NMD sfr
//-----------------------------------------------------------------------------
#define  WRAPPING          0x1
#define  NO_WRAPPING       0x0

//-----------------------------------------------------------------------------
// defines used with DMA0NCF sfr
//-----------------------------------------------------------------------------
#define  DMA_BIG_ENDIAN    0x10
#define  DMA_INT_EN        0x80

//-----------------------------------------------------------------------------
// miscellaneous defines used for AES0BCF
//-----------------------------------------------------------------------------
#define DECRYPTION_MODE          0x00
#define ENCRYPTION_MODE          0x04
#define AES_ENABLE               0x08

//-----------------------------------------------------------------------------
// defines used for AES0DCF
//-----------------------------------------------------------------------------
#define AES_OUTPUT               0x00
#define XOR_ON_INPUT             0x01
#define XOR_ON_OUTPUT            0x02
#define INVERSE_KEY              0x04

//-----------------------------------------------------------------------------
// Define SUCCESS and ERROR_INVALID_PARAMETER return codes
//-----------------------------------------------------------------------------
#ifndef SUCCESS
#define SUCCESS 0
#elif(SUCCESS!=0)
#error  "SUCCESS definition conflict!"
#endif
#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER 1
#elif(ERROR_INVALID_PARAMETER!=1)
#error  "ERROR_INVALID_PARAMETER definition conflict!"
#endif

#endif  // CPU_SI1030
#endif	// _AES_ENCRYPT_H_
