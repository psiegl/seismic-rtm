# Copyright 2017 - , Dr.-Ing. Patrick Siegl
# SPDX-License-Identifier: BSD-2-Clause

UNAME_M := $(shell uname -m)

SDIR    := src
BDIR    := bld.$(UNAME_M)

TARGET  := seismic-rtm.elf


default: compile run

compile:
	mkdir -p $(BDIR)
	cd $(BDIR) && cmake ..
	make -C $(BDIR)


WIDTH   = 1000
HEIGHT  = 516
PULSEX  = 600
PULSEY  = 70
STEPS   = 1000
THREADS = 8
TYPE    = plain_opt

CMD     = --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE)

run: compile
	./$(BDIR)/$(TARGET) $(CMD)

bench: compile
	ln -sf $(BDIR)/$(TARGET) $(TARGET)
	python tools/bench.py

ascii: compile
	./$(BDIR)/$(TARGET) $(CMD) --ascii=1

visualize: compile
	./$(BDIR)/$(TARGET) $(CMD) --output
	./tools/ximage.$(UNAME_M).elf n1=$(HEIGHT) n2=$(WIDTH) hbox=$(HEIGHT) wbox=$(WIDTH) title=visualizer < output.bin

objdump: compile
	objdump -dS $(BDIR)/$(TARGET) | less

clean:
	rm $(BDIR) *.bin -rf

distclean: clean
	rm seismic.* -rf
