#  This file is part of seismic-rtm.
#
#  seismic-rtm is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  seismic-rtm is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with seismic.  If not, see <http://www.gnu.org/licenses/>.


UNAME_P := $(shell uname -p)

SDIR    := src
BDIR    := bld.$(UNAME_P)

TARGET  := seismic.$(UNAME_P).elf

OBJS    := $(BDIR)/config.o $(BDIR)/main.o $(BDIR)/visualize.o
CHK_HW  := $(BDIR)/check_hw.o
PLAIN   := $(BDIR)/kernel/kernel_plain.o

CC      = gcc # clang
CFLAGS  = -ffast-math -ffp-contract=fast -Ofast #-march=native

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
