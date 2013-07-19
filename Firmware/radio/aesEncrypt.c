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
/// @file	aesEncrypt.c
///
/// DMA - AES Hardware Encryption.
///

//=============================================================================
// Includes
//=============================================================================
#include "aesEncrypt.h"
//=============================================================================
// local function prototypes
//=============================================================================
#ifdef INCLUDE_ENCRYPTION

void IncrementCounter(__xdata counter);

uint8_t CTR_EncryptDecrypt (uint8_t operation, uint8_t __xdata *plainText, uint8_t __xdata *cipherText,
							uint8_t __xdata *counter, uint8_t __xdata *encryptionKey, uint16_t blocks)
{
	// Unions used for compiler independent endianness.
	uint16_t length;                        // Length in bytes for all blocks.
	uint16_t addr;                          // Union used to access pointer bytes.
	
	uint8_t keyLength;                       // Used to calculate key length in bytes.
	
	// Check first for valid operation.
	if((operation == DECRYPTION_UNDEFINED)||(operation >= ENCRYPTION_UNDEFINED))
	{
		return ERROR_INVALID_PARAMETER;
	}
	else
	{
		// Calculate key length in bytes based on operation parameter.
		keyLength = (((operation & 0x03) + 2) << 3);
	}
	
	// Calculate plaintext and ciphertext total length.
	// Using <<4 in lieu of * 16 for code efficiency.
	length = (blocks << 4);
	
	// From this point on, "blocks" is used to count remaining blocks.
	blocks--;
	
	SFRPAGE = DPPE_PAGE; // Change the Special Function Register Page (P236 sil102x_103x_ds.pdf)
	
	AES0BCFG = 0x00;		// disable Block Config, for now
	AES0DCFG = 0x00;		// disable Data Config, for now
	
	// Disable AES0KIN, AES0BIN, AES0XIN, & AES0YOUT channels.
	DMA0EN &= ~AES0_KBXY_MASK;
	
	// Configure AES key input channel using key pointer.
	// Set length to calculated key length.
	// Set DMA0NMD to disable Key wrapping.
	// This is necessary because we want the Key DMA channel
	// to stop after the first block.
	
	addr = (uint16_t)(encryptionKey);
	DMA0SEL = AES0KIN_CHANNEL;
	DMA0NCF = AES0KIN_PERIPHERAL_REQUEST;
	DMA0NMD = NO_WRAPPING;
	DMA0NBAL = addr&0xff;
	DMA0NBAH = addr>>8;
	DMA0NSZH = 0;
	DMA0NSZL = keyLength;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// AES block input is always the counter value for both CTR encryption
	// and decryption. Set length to 16 for first block only.
	// Clear DMA0NMD to disable counter wrapping.
	// This is necessary because we want the Counter DMA channel
	// to stop after the first block.
	
	DMA0SEL = AES0BIN_CHANNEL;
	DMA0NCF = AES0BIN_PERIPHERAL_REQUEST;
	DMA0NMD = NO_WRAPPING;
	addr = (uint16_t)(counter);
	DMA0NBAL = addr&0xff;
	DMA0NBAH = addr>>8;
	DMA0NSZL = 16;                      // one block
	DMA0NSZH = 0;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// AES XOR input is plaintext for encryption operation or ciphertext
	// for decryption operation.
	
	if(operation & ENCRYPTION_MODE)
		addr = (uint16_t)(plainText);
	else
		addr = (uint16_t)(cipherText);
	
	// Configure AES XOR input channel using corresponding address.
	// Clear DMA0NMD to disable wrapping.
	
	DMA0SEL = AES0XIN_CHANNEL;
	DMA0NCF = AES0XIN_PERIPHERAL_REQUEST;
	DMA0NMD = NO_WRAPPING;
	DMA0NBAL = addr&0xff;
	DMA0NBAH = addr>>8;
	DMA0NSZL = 16;                      // one block
	DMA0NSZH = 0;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// AES Y output is ciphertext  for encryption operation or
	// plaintext for decryption operation.
	
	if(operation & ENCRYPTION_MODE)
		addr = (uint16_t)(cipherText);
	else
		addr = (uint16_t)(plainText);
	
	// Configure AES Y output channel using corresponding address
	// Set length to 16 for first block only.
	// Clear DMA0NMD to disable wrapping.
	
	DMA0SEL = AES0YOUT_CHANNEL;
	DMA0NCF = AES0YOUT_PERIPHERAL_REQUEST|DMA_INT_EN;
	DMA0NMD = NO_WRAPPING;
	
	DMA0NBAL = addr&0xff;
	DMA0NBAH = addr>>8;
	DMA0NSZL = 16;                      // one block
	DMA0NSZH = 0;
	DMA0NAOH = 0;
	DMA0NAOL = 0;
	
	// Clear KBXY (Key, Block, X in, and Y out) bits in DMA0INT sfr using mask.
	DMA0INT &= ~AES0_KBXY_MASK;
	
	
	// Set KBXY (Key, Block, Xin, and Y out) bits in DMA0EN sfr using mask.
	// This enables the DMA. But the encryption/decryption operation
	// won't start until the AES block is enabled.
	DMA0EN  |=  AES0_KBXY_MASK;
	
	// Configure data path for XOR on output. CTR mode always has the
	// XOR on the output for both encryption and decryption.
	AES0DCFG = XOR_ON_OUTPUT;
	
	// Configure AES0BCFG for encryption/decryption and requested key size.
	// Note the encryption mode bit is set explicitly.
	AES0BCFG = operation;
	AES0BCFG |= ENCRYPTION_MODE;
	AES0BCFG |= AES_ENABLE;
	
    // Enable DMA interrupt to terminate Idle mode.
    EIE2 |= 0x20;
	
	// This do...while loop ensures that the CPU will remain in Idle mode
	// until AES0YOUT DMA channel transfer is complete.
	do
	{
#ifdef DMA_TRANSFERS_USE_IDLE
		PCON |= 0x01;                    // go to Idle mode
#endif
	}  while((DMA0INT&AES0YOUT_MASK)==0);
	
	while(blocks--)                     // if blocks remaining
	{
		// It is necessary to either pause the AES DMA channels
		// or disable the AES block while changing the setup.
		// Both steps are taken in this example to be extra safe.
		
		// Disable AES block.
		// This also clears the AES contents and resets the state machine.
		// It is not necessary to reset the AES core between blocks.
		// But it is recommended to disable the core while changing the set-up.
		
		AES0BCFG &= ~AES_ENABLE;
		
		// Pause DMA channels used by AES block.
		DMA0EN &= ~AES0_KBXY_MASK;
		
		IncrementCounter(counter);
		
		SFRPAGE = DPPE_PAGE;
		
		// AESKIN DMA channel reset address offset
		DMA0SEL = AES0KIN_CHANNEL;
		DMA0NAOL = 0;
		DMA0NAOH = 0;
		
		// AESBIN DMA channel reset address offset
		DMA0SEL = AES0BIN_CHANNEL;
		DMA0NAOL = 0;
		DMA0NAOH = 0;
		
		// Ch 2 AES0XIN increment size by 16 bytes
		DMA0SEL = AES0XIN_CHANNEL;
		length.U8[LSB] = DMA0NSZL;
		length.U8[MSB] = DMA0NSZH;
		length.U16 += 16;
		DMA0NSZL = length.U8[LSB];
		DMA0NSZH = length.U8[MSB];
		
		// Ch 3 AESYOUT increment size by 16 bytes
		DMA0SEL = AES0YOUT_CHANNEL;
		length.U8[LSB] = DMA0NSZL;
		length.U8[MSB] = DMA0NSZH;
		length.U16 += 16;
		DMA0NSZL = length.U8[LSB];
		DMA0NSZH = length.U8[MSB];
		
		// Clear KBXY (Key, Block, X in, and Y out) bits in DMA0INT sfr using mask.
		DMA0INT &= ~AES0_KBXY_MASK;
		
		// Set KBXY (Key, Block, Xin, and Y out) bits in DMA0EN sfr using mask.
		// This enables the DMA. But the encryption/decryption operation
		// won't start until the AES block is enabled.
		DMA0EN  |=  AES0_KBXY_MASK;
		
		// Enabled AES module to start encryption/decryption operation.
		AES0BCFG |= AES_ENABLE;
		
		// enable DMA interrupt to terminate Idle mode
		EIE2 |= 0x20;
		
		// This do...while loop ensures that the CPU will remain in Idle mode
		// until AES0YOUT DMA channel transfer is complete.
		do
		{
#ifdef DMA_TRANSFERS_USE_IDLE
			PCON |= 0x01;                 // go to Idle mode
#endif
		}  while((DMA0INT & AES0YOUT_MASK)==0);
	}
	
	//Clear AES Block
	AES0BCFG = 0x00;
	AES0DCFG = 0x00;
	
	// Clear KXBY (Key, Block, XOR, and Y out) bits in DMA0EN sfr using mask.
	DMA0EN &= ~AES0_KBXY_MASK;
	
	// Clear KBY (Key, Block, and Y out) bits in DMA0INT sfr using mask.
	DMA0INT &= ~AES0_KBXY_MASK;
	
	return SUCCESS;
}
//-----------------------------------------------------------------------------
// IncrementCounter()
//
// This function is used to increment the 16-byte counter.
//
// This version is hand optimized to produce efficient assembler.
// This results in obtuse C code.
//
//-----------------------------------------------------------------------------
void IncrementCounter (__xdata counter)
{
	uint8_t i;
	uint8_t x;
	
	i = 16;
	counter += 16;					// point to end of data
	
	do
	{
		counter--;					// decrement data pointer
		x = *counter;				// read xdata using data pointer
		x++;						// increment value
		*counter = x;				// move to xram
		if(x) break;				// break if not zero
	}  while(--i);					// DJNZ
}
#else // INCLUDE_ENCRYPTION
void noEncryption() {}

#endif // INCLUDE_ENCRYPTION