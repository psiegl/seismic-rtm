#  This file is part of seismic-rtm.
#
#  seismic-rtm is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  seismic is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

TARGET := seismic.elf
OBJS   := kernel_avx_unaligned.o kernel_avx_fma_unaligned.o kernel_avx2_unaligned.o kernel_avx2_fma_unaligned.o kernel_sse_unaligned.o kernel_sse_aligned_not_grouped.o kernel_sse_aligned.o kernel_sse_std.o kernel_plain.o config.o main.o visualize.o

WIDTH   = 1000
HEIGHT  = 516
PULSEX  = 600
PULSEY  = 70
STEPS   = 1000
THREADS = 8
TYPE    = sse_unaligned
CC      = clang #gcc

default: compile run

%.elf: $(OBJS)
	$(CC) -pthread -lm -o $@ $^

# we could use -march=native, but we would always compile out not supported ISA features, which means we would need to recompile for any processor...
%.o: %.c
	$(CC) -O3 -ffast-math -msse -mavx -mavx2 -mfma -c -o $@ $<

compile: $(TARGET)

run:
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE)

ascii: compile
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE) --ascii=1

visualize: compile
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE) --output
	./ximage_x86 n1=$(HEIGHT) n2=$(WIDTH) hbox=$(HEIGHT) wbox=$(WIDTH) title=visualizer < output.bin

objdump: compile
	objdump -dS $(TARGET) | less

clean:
	rm $(TARGET) *.o *.bin -rf
