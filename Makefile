#  This file is part of seismic-rtm.
#
# Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


UNAME_P := $(shell uname -m)

SDIR    := src
BDIR    := bld.$(UNAME_P)

TARGET  := seismic.$(UNAME_P).elf

OBJS    := $(BDIR)/config.o $(BDIR)/main.o $(BDIR)/visualize.o
CHK_HW  := $(BDIR)/check_hw.o
PLAIN   := $(BDIR)/kernel/kernel_plain.o

CC      = gcc #clang
CFLAGS  = -ffast-math -ffp-contract=fast -Ofast -fprefetch-loop-arrays  #-ggdb -march=native

ifeq ($(UNAME_P),x86_64)
 include makerules.x86
else
 ifneq (,$(filter $(UNAME_P),armv7l aarch64))
  include makerules.arm
 else
  include makerules.ppc64
 endif
endif
ALL_OBJS = $(OBJS) $(CHK_HW) $(PLAIN) $(ADD_KERNELS)

default: compile run

$(BDIR):
	mkdir -p $(BDIR)/kernel

%.elf: $(ALL_OBJS)
	$(CC) -pthread -lm -o $@ $^

#.INTERMEDIATE: $(ALL_OBJS)
$(ALL_OBJS): $(BDIR)/%.o: $(SDIR)/%.c
	$(CC) -I$(SDIR) $(CFLAGS) -c -o $@ $<


compile: $(BDIR) $(TARGET)

WIDTH   = 1000
HEIGHT  = 516
PULSEX  = 600
PULSEY  = 70
STEPS   = 1000
THREADS = 8
TYPE    = plain_opt

CMD     = --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE)

run: compile
	./$(TARGET) $(CMD)

bench: compile
	python tools/bench.py

ascii: compile
	./$(TARGET) $(CMD) --ascii=1

visualize: compile
	./$(TARGET) $(CMD) --output
	./tools/ximage.$(shell uname -m).elf n1=$(HEIGHT) n2=$(WIDTH) hbox=$(HEIGHT) wbox=$(WIDTH) title=visualizer < output.bin

objdump: compile
	objdump -dS $(TARGET) | less

clean:
	rm $(TARGET) $(BDIR) *.bin -rf

distclean: clean
	rm bld.* seismic.* -rf
