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

#ifdef INCLUDE_ENCRYPTION

SEGMENT_VARIABLE (DecryptionKey[32], U8, SEG_XDATA);
SEGMENT_VARIABLE (Counter[16], U8, SEG_XDATA);
__pdata uint8_t KeySize;

//-----------------------------------------------------------------------------
// Nonce used for counter initial value
//-----------------------------------------------------------------------------
const SEGMENT_VARIABLE (Nonce[16], U8, SEG_CODE) =
{0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

//=============================================================================
// Function Prototypes (API)
//=============================================================================

GENERATE_DECRYPTION_KEY_STATUS
GenerateDecryptionKey (VARIABLE_SEGMENT_POINTER(encryptionKey, U8, SEG_XDATA),
					   VARIABLE_SEGMENT_POINTER(decryptionKey, U8, SEG_XDATA),
					   U8 GENERATE_DECRYPTION_KEY_SIZE) __nonbanked;

uint8_t
CTR_EncryptDecrypt (__pdata uint8_t operation,
					VARIABLE_SEGMENT_POINTER(plainText, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(cipherText, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(counter, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(encryptionKey, uint8_t, SEG_XDATA),
					__pdata uint16_t blocks) __nonbanked;

void initCounter (VARIABLE_SEGMENT_POINTER(counter, U8, SEG_XDATA),
				  VARIABLE_SEGMENT_POINTER(nonce, U8, SEG_CODE))__nonbanked;

void IncrementCounter(VARIABLE_SEGMENT_POINTER(counter, uint8_t, SEG_XDATA)) __nonbanked;

//=============================================================================
// local function prototypes
//=============================================================================

void aesEncrypt_init(){
	__pdata uint16_t encyptionBits = (uint16_t)(param_get(PARAM_ENCRYPTION));
	KeySize = 0;
		
	if(encyptionBits > 0)
	{
		switch (encyptionBits)
		{
			case 128:
				if(GenerateDecryptionKey(EncryptionKey, DecryptionKey, KEY_SIZE_128_BITS) == SUCCESS) {
					KeySize = KEY_SIZE_128_BITS;
					return;
				}
				break;
				
			case 192:
				if(GenerateDecryptionKey(EncryptionKey, DecryptionKey, KEY_SIZE_192_BITS) == SUCCESS) {
					KeySize = KEY_SIZE_192_BITS;
					return;
				}
				break;
				
			case 256:
				if(GenerateDecryptionKey(EncryptionKey, DecryptionKey, KEY_SIZE_256_BITS) == SUCCESS) {
					KeySize = KEY_SIZE_256_BITS;
					return;
				}
				break;
				
			default:
				// ERROR..
				break;
		}
		// If we reached here there must have been a error, lets turn encyption off..
		printf("Error: Unable to generate DecryptionKey from EncryptionKey - Turning Encyption off\r\n");
		param_set(PARAM_ENCRYPTION, 0);
	}
}

uint8_t aesEncryptDecrypt(__pdata uint8_t operation,
						  VARIABLE_SEGMENT_POINTER(plainText, uint8_t, SEG_XDATA),
						  VARIABLE_SEGMENT_POINTER(cipherText, uint8_t, SEG_XDATA),
						  __pdata uint16_t blocks)
{	
	if(KeySize > 0)
	{
		if(true) // Use CTR
		{
			initCounter(Counter, Nonce);
			if(operation == ENCRYPTION)
			{
				// Add 4 to the key size to place it into encryption mode.
				return CTR_EncryptDecrypt (KeySize+4, plainText, cipherText, Counter, EncryptionKey, blocks);
			}
			else // DECRYPTION
			{
				return CTR_EncryptDecrypt (KeySize, plainText, cipherText, Counter, EncryptionKey, blocks);
			}
		}
	}
	// No encryption used
	return 0;
}

uint8_t
CTR_EncryptDecrypt (__pdata uint8_t operation,
					VARIABLE_SEGMENT_POINTER(plainText, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(cipherText, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(counter, uint8_t, SEG_XDATA),
					VARIABLE_SEGMENT_POINTER(encryptionKey, uint8_t, SEG_XDATA),
					__pdata uint16_t blocks) __nonbanked
{
	// Unions used for compiler independent endianness.
	UU16 length;                        // Length in bytes for all blocks.
	UU16 addr;                          // Union used to access pointer bytes.
	
	__pdata uint8_t keyLength;                       // Used to calculate key length in bytes.
	
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
	length.U16 = (blocks << 4);
	
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
	
	addr.U16 = (uint16_t)(encryptionKey);
	DMA0SEL = AES0KIN_CHANNEL;
	DMA0NCF = AES0KIN_PERIPHERAL_REQUEST;
	DMA0NMD = NO_WRAPPING;
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
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
	addr.U16 = (uint16_t)(counter);
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
	DMA0NSZL = 16;                      // one block
	DMA0NSZH = 0;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// AES XOR input is plaintext for encryption operation or ciphertext
	// for decryption operation.
	
	if(operation & ENCRYPTION_MODE)
		addr.U16 = (uint16_t)(plainText);
	else
		addr.U16 = (uint16_t)(cipherText);
	
	// Configure AES XOR input channel using corresponding address.
	// Clear DMA0NMD to disable wrapping.
	
	DMA0SEL = AES0XIN_CHANNEL;
	DMA0NCF = AES0XIN_PERIPHERAL_REQUEST;
	DMA0NMD = NO_WRAPPING;
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
	DMA0NSZL = 16;                      // one block
	DMA0NSZH = 0;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// AES Y output is ciphertext  for encryption operation or
	// plaintext for decryption operation.
	
	if(operation & ENCRYPTION_MODE)
		addr.U16 = (uint16_t)(cipherText);
	else
		addr.U16 = (uint16_t)(plainText);
	
	// Configure AES Y output channel using corresponding address
	// Set length to 16 for first block only.
	// Clear DMA0NMD to disable wrapping.
	
	DMA0SEL = AES0YOUT_CHANNEL;
	DMA0NCF = AES0YOUT_PERIPHERAL_REQUEST|DMA_INT_EN;
	DMA0NMD = NO_WRAPPING;
	
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
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
void IncrementCounter (VARIABLE_SEGMENT_POINTER(counter, uint8_t, SEG_XDATA)) __nonbanked
{
	__pdata uint8_t i;
	__pdata uint8_t x;
	
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

//-----------------------------------------------------------------------------
// GenerateDecryptionKey()
//
// parameters:
//    encryptionKey     - xdata pointer to encryption key data source
//    decryptionKey     - xdata pointer to decryption key data destination
//    keySize           - enumerated type for key size 0/1/2 = 128/192/256
//
// returns:
//    status            - 0 for success
//                      - 1 for ERROR - Invalid Key size parameter.
//
// description:
//
// This function will generate a decryption key corresponding to a
// given encryption key.
//
// The encryption key must be stored in xdata prior to calling
// the GenerateDecryptionKey function.
//
// The encryption key must be used for all encryption operations and the
// decryption key must be used for all decryption operations.
// (Except CTR Mode which uses the encryption key for both operations.)
//
// The AES code examples uses four channels as defined in the DMA_defs.h file.
// The channel assignments are static and fixed at compile time. They are
// not assigned dynamically.
//
// This function puts the MCU core into idle mode while encryption operation
// is in process. This function is blocking. The function does not return
// until after the encrpytion operation has completed.
//
// A interrupt is needed to wake up from Idle mode. This requires that the
// global interrupts are enabled. Also, a small DMA ISR that disables EIE2
// bit 5 is required in the main module.
//
// Note that the extended portion of the extended keys comes out first,
// before the first 16 bytes. So two DMA operations are used for the
// AESYOUT channel for an extended key.
//
//-----------------------------------------------------------------------------
GENERATE_DECRYPTION_KEY_STATUS
GenerateDecryptionKey (
					   VARIABLE_SEGMENT_POINTER(encryptionKey, U8, SEG_XDATA),
					   VARIABLE_SEGMENT_POINTER(decryptionKey, U8, SEG_XDATA),
					   GENERATE_DECRYPTION_KEY_SIZE keySize) __nonbanked
{
	// unions used for compiler independent endianness.
	UU16 addr;                          // Union used to access pointer bytes.
	
	U8 keyLength;                       // Used to calculate key length in bytes.
	
	// check first for valid operation
	if(keySize >= KEY_SIZE_UNDEFINED)
	{
		return ERROR_INVALID_PARAMETER;
	}
	else
	{
		// Calculate key length in bytes based on operation parameter.
		keyLength = ((keySize + 2) << 3);
	}
	
	SFRPAGE = DPPE_PAGE;
	
	AES0BCFG = 0x00;                      // disable, for now
	AES0DCFG = 0x00;                      // disable for now
	
	// Disable AES0KIN, AES0BIN, & AES0YOUT channels.
	DMA0EN &= ~AES0_KBY_MASK;
	
	// Configure AES key input channel using key pointer.
	// Set length to calculated key length.
	// Clear DMA0NMD to disable Key wrapping.
	// Generating the decryption key only requires encrypting one block.
	DMA0SEL  = AES0KIN_CHANNEL;
	DMA0NCF  = AES0KIN_PERIPHERAL_REQUEST;
	DMA0NMD  = NO_WRAPPING;
	addr.U16 = (U16)(encryptionKey);
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
	DMA0NSZH = 0;
	DMA0NSZL = keyLength;
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// Configure AES Block input channel.
	// Since the input data does not matter, the base address is set to
	// 0x0000 and the first 16 bytes of data in xram are used.
	// Set length to 16.
	// Clear DMA0NMD to disable Key wrapping.
	DMA0SEL  = AES0BIN_CHANNEL;
	DMA0NCF  = AES0BIN_PERIPHERAL_REQUEST;
	DMA0NMD  = NO_WRAPPING;
	DMA0NBAL = 0x00;
	DMA0NBAH = 0x00;
	DMA0NSZH = 0;
	DMA0NSZL = 16;                       // one block
	DMA0NAOL = 0;
	DMA0NAOH = 0;
	
	// Configure AES Y output channel for decryption key.
	// Enable DMA interrupt
	// Set length to calculated key length.
	// Clear DMA0NMD to disable Key wrapping.
	DMA0SEL = AES0YOUT_CHANNEL;
	DMA0NCF = AES0YOUT_PERIPHERAL_REQUEST|DMA_INT_EN;
	DMA0NMD = NO_WRAPPING;
	addr.U16 = (U16)(decryptionKey);
	DMA0NBAL = addr.U8[LSB];
	DMA0NBAH = addr.U8[MSB];
	DMA0NSZH = 0;
	DMA0NSZL = keyLength;
	DMA0NAOH = 0;
	
	// Configure starting address offset to byte 16 if extended key is used.
	if (keySize> KEY_SIZE_128_BITS)     // if extended key
		DMA0NAOL = 0x10;                 // point to extended portion
	else
		DMA0NAOL = 0x00;                 // else point to start
	
	// Clear KBY (Key, Block, and Y out) bits in DMA0INT sfr using mask.
	DMA0INT &= ~AES0_KBY_MASK;
	// Set KBY (Key, Block, and Y out) bits in DMA0EN sfr using mask.
	// This enables the DMA. But the encyption/decryption operation
	// won't start until the AES block is enabled.
	DMA0EN  |=  AES0_KBY_MASK;
	
	// Configure AES0DCFG to send inverse key to AES0YOUT
	AES0DCFG = INVERSE_KEY;
	
	// Set AES0BCFG according to keySize parameter.
	AES0BCFG = keySize;
	
	// Generating the decryption key always uses encryption mode.
	AES0BCFG |= ENCRYPTION_MODE;
	
	// Enabled AES module to start encryption operation.
	AES0BCFG |= AES_ENABLE;
	
	// enable DMA interrupt to terminate Idle mode
	EIE2 |= 0x20;
	
	
	// This do...while loop ensures that the CPU will remain in Idle mode
	// until AES0YOUT DMA channel transfer is complete.
	do
	{
#ifdef DMA_TRANSFERS_USE_IDLE
		PCON |= 0x01;                    // go to Idle mode
#endif
	}  while((DMA0INT & AES0YOUT_MASK)==0);
	
	if (keySize> KEY_SIZE_128_BITS)     // if extended key
	{
		// It is reccomended to pause the AES DMA channels
		// while changing the DMA setup. The AES block cannot
		// be disabled or the rest of the key would be lost.
		
		// Pause DMA channels used by AES block.
		DMA0EN &= ~AES0_KBY_MASK;
		
		DMA0SEL = AES0YOUT_CHANNEL;      // select AES0YOUT DMA ch
		DMA0NAOL = 0;                    // reset pointer
		DMA0NSZL = 0x10;                 // set length to 16 (128-bits)
		
		// Clear KBXY (Key, Block, and Y out) bits in DMA0INT sfr using mask.
		DMA0INT &= ~AES0_KBXY_MASK;
		
		// Set AES0YOUT DMA enable bit only in DMA0EN sfr using mask.
		// This enables the DMA and the rest of the key will be written out.
		DMA0EN  |=  AES0YOUT_MASK;
		
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
	
	// Clear KBY (Key, Block, and Y out) bits in DMA0EN sfr using mask.
	DMA0EN &= ~AES0_KBY_MASK;
	// Clear KBY (Key, Block, and Y out) bits in DMA0INT sfr using mask.
	DMA0INT &= ~AES0_KBY_MASK;
	
	return SUCCESS;
}

//-----------------------------------------------------------------------------
// initCounter()
//
// parameters:
//   counter      - xdata pointer to counter
//   nonce        - code pointer to nonce
//
// returns:
//
// description:
//
// Copies nonce in Flash to counter in xdata RAM. Nonce is a crpytographic
// term. The nonce is a number used once. It is used for the initial value of
// the counter. The counter must be stored to xdata for use by the DMA.
//
//-----------------------------------------------------------------------------
void initCounter (VARIABLE_SEGMENT_POINTER(counter, U8, SEG_XDATA),
				  VARIABLE_SEGMENT_POINTER(nonce, U8, SEG_CODE)) __nonbanked
{
	U8 i;
	
	for(i=16;i>0;i--)
	{
		*counter++ = *nonce++;
	}
}

#else // INCLUDE_ENCRYPTION
// Dummpy function to stop the compiler complaining when this file is not used
void noEncryption() {}

#endif // INCLUDE_ENCRYPTION