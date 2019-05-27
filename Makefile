# Copyright 2017 - , Dr.-Ing. Patrick Siegl
# SPDX-License-Identifier: BSD-2-Clause

UNAME_M := $(shell uname -m)

SDIR    := src
BDIR    := bld.$(UNAME_M)

TARGET  := seismic.$(UNAME_M).elf

OBJS    := $(BDIR)/config.o $(BDIR)/main.o $(BDIR)/visualize.o
CHK_HW  := $(BDIR)/check_hw.o
PLAIN   := $(BDIR)/kernel/kernel_plain.o

CC      = gcc #clang
CFLAGS  = -ffast-math -ffp-contract=fast -Ofast -fprefetch-loop-arrays  #-ggdb -march=native

ifeq ($(UNAME_M),x86_64)
 include makerules.x86
else
 ifneq (,$(filter $(UNAME_M),armv6l armv7l aarch64))
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
	$(CC) -pthread -o $@ $^ -lm

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
	./tools/ximage.$(UNAME_M).elf n1=$(HEIGHT) n2=$(WIDTH) hbox=$(HEIGHT) wbox=$(WIDTH) title=visualizer < output.bin

objdump: compile
	objdump -dS $(TARGET) | less

clean:
	rm $(TARGET) $(BDIR) *.bin -rf

distclean: clean
	rm bld.* seismic.* -rf
