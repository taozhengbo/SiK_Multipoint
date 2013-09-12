// -*- Mode: C; c-basic-offset: 8; -*-
//
// Copyright (c) 2013 Luke Hovington, All Rights Reserved
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
/// @file	pins_user.c
///
/// AT Controlled Pins
///

#include "pins_user.h"

//P2MDOUT

__code const struct pins_user_map {
	uint8_t port;
	uint8_t	pin;
} pins_user_map[] = {
// Pin rfd900a  Mapping
#ifdef BOARD_rfd900a
	{2, 3}, // 0 - P2.3
	{2, 2}, // 1 - P2.2
	{2, 1}, // 2 - P2.1
	{2, 0}, // 3 - P2.0
	{2, 6}, // 4 - P2.6
	{0, 1}, // 5 - P0.1
};
#else
};
#endif

void
pins_user_init(void)
{
	__pdata uint8_t i;
	// Set the Default pin behaviour
	for(i=0; i<PIN_MAX; i++)
	{
		pins_user_set_io(i, pin_values[i].output);
		pins_user_set_direction(i, PIN_LOW);
	}
}

bool
pins_user_set_io(__pdata uint8_t pin, __pdata uint8_t in_out)
{
	if (PIN_MAX > pin)
	{
		pin_values[pin].output = in_out;
		pin_values[pin].pin_mirror = PIN_NULL;
		
//      // Analog support for later implemetation?
//		SFRPAGE	= LEGACY_PAGE;
//		
//		switch(pins_user_map[pin].port)
//		{
//			case 0:
//				if(in_out)
//				{
//					P0MDIN |= (1<<pins_user_map[pin].pin);
//				}
//				else
//				{
//					P0MDIN &= ~(1<<pins_user_map[pin].pin);
//				}
//				break;
//				
//			case 1:
//				if(in_out)
//				{
//					P1MDIN |= (1<<pins_user_map[pin].pin);
//				}
//				else
//				{
//					P1MDIN &= ~(1<<pins_user_map[pin].pin);
//				}
//				break;
//				
//			case 2:
//				if(in_out)
//				{
//					P2MDIN |= (1<<pins_user_map[pin].pin);
//				}
//				else
//				{
//					P2MDIN &= ~(1<<pins_user_map[pin].pin);
//				}
//				break;
//				
//			default:
//				return false;
//		}
		pins_user_set_direction(pin, PIN_LOW);
		return true;
	}
	return false;
}

bool
pins_user_set_direction(__pdata uint8_t pin, __pdata uint8_t high_low)
{
	if(PIN_MAX > pin && pin_values[pin].output == PIN_OUTPUT && pin_values[pin].pin_mirror == PIN_NULL)
	{
		switch(pins_user_map[pin].port)
		{
			case 0:
				if(high_low)
				{
					P0 |= (1<<pins_user_map[pin].pin);
				}
				else
				{
					P0 &= ~(1<<pins_user_map[pin].pin);
				}
				break;
			
			case 1:
				if(high_low)
				{
					P1 |= (1<<pins_user_map[pin].pin);
				}
				else
				{
					P1 &= ~(1<<pins_user_map[pin].pin);
				}
				break;
				
			case 2:
				if(high_low)
				{
					P2 |= (1<<pins_user_map[pin].pin);
				}
				else
				{
					P2 &= ~(1<<pins_user_map[pin].pin);
				}
				break;
				
			default:
				return false;
		}
		return true;
	}
	return false;
}

uint8_t
pins_user_get_direction(__pdata uint8_t pin)
{
	if(PIN_MAX > pin && pin_values[pin].output == PIN_INPUT)
	{
		switch(pins_user_map[pin].port)
		{
			case 0:
				return P0 & (1<<pins_user_map[pin].pin)?1:0;
				break;
				
			case 1:
				return P1 & (1<<pins_user_map[pin].pin)?1:0;
				break;
				
			case 2:
				return P2 & (1<<pins_user_map[pin].pin)?1:0;
				break;
				
			default:
				return 0x55;
		}
	}
	return 0x55;
}
