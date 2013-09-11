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

//void
//pins_user_init()
//{
////	// Set the Default pin behaviour
////#ifdef PIN_0
////	__pdata struct pins_info* pin_data;
////	pin_data = param_get_pins();
////#endif // PIN_0
//}

bool
pins_user_set_io(__pdata uint8_t pin, __pdata uint8_t in_out)
{
	if (PIN_MAX() > pin)
	{
		pin_values[pin].output = in_out;
		pin_values[pin].pin_mirror = PIN_NULL;
		return true;
	}
	return false;
}

bool
pins_user_set_direction(__pdata uint8_t pin, __pdata uint8_t high_low)
{
#if PIN_MAX() > 0
	if(PIN_MAX() > pin && pin_values[pin].output == PIN_OUTPUT && pin_values[pin].pin_mirror == PIN_NULL)
	{
		high_low = high_low>0?true:false;
		switch (pin) {
			case 0:
				PIN_0 = high_low;
				break;
#if PIN_MAX() > 1
			case 1:
				PIN_1 = high_low;
				break;
#endif // PIN_1
#if PIN_MAX() > 2
			case 2:
				PIN_2 = high_low;
				break;
#endif // PIN_2
#if PIN_MAX() > 3
			case 3:
				PIN_3 = high_low;
				break;
#endif // PIN_3
#if PIN_MAX() > 4
			case 4:
				PIN_4 = high_low;
				break;
#endif // PIN_4
#if PIN_MAX() > 5
			case 5:
				PIN_5 = high_low;
				break;
#endif // PIN_5
#if PIN_MAX() > 6
			case 6:
				PIN_6 = high_low;
				break;
#endif // PIN_6
#if PIN_MAX() > 7
			case 7:
				PIN_7 = high_low;
				break;
#endif // PIN_7
#if PIN_MAX() > 8
			case 8:
				PIN_8 = high_low;
				break;
#endif // PIN_8
#if PIN_MAX() > 9
			case 9:
				PIN_9 = high_low;
				break;
#endif // PIN_9
			default:
				return false;
				break;
		}
		return true;
	}
#else // PIN_0
	printf("No User Defined Pins %d:%d\n", pin, high_low>0?true:false);
#endif // PIN_0
	return false;
}
