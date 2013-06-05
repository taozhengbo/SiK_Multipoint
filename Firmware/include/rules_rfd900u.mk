#
# Copyright (c) 2011 Michael Smith, All Rights Reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  o Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  o Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
# Configuration options for the RFD900U board.
#
FREQUENCIES			 = 915
XRAM_SIZE			 = 8448
HAVE_BANKING		 = 1
BANKS				 = -Wl-bBANK1=0x18000 -Wl-bBANK2=0x28000 -Wl-bBANK3=0x38000 -Wl-r
LDFLAGS				+= --model-huge --iram-size 256 --xram-size $(XRAM_SIZE) --code-loc 0x400 --code-size 0x1F400 --stack-size 64 $(BANKS)
BOOTLDFLAGS			 = --iram-size 256 --xram-size $(XRAM_SIZE) --stack-size 64 --nostdlib -Wl-bHIGHCSEG=0x3FC00 -Wl-r
