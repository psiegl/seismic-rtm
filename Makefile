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

TARGET := seismic.elf
OBJS   := kernel_plain.o config.o main.o visualize.o

SSE    := kernel_sse.o kernel_sse_fma.o kernel_sse_avx_fma_partial_aligned.o kernel_sse_avx_partial_aligned.o
AVX    := kernel_avx.o kernel_avx_fma.o kernel_avx2.o kernel_avx2_fma.o kernel_sse_avx_fma_partial_aligned.o kernel_sse_avx_partial_aligned.o
AVX2   := kernel_avx2.o kernel_avx2_fma.o
FMA    := kernel_avx_fma.o kernel_sse_fma.o kernel_avx2_fma.o kernel_sse_avx_fma_partial_aligned.o


WIDTH   = 1000
HEIGHT  = 516
PULSEX  = 600
PULSEY  = 70
STEPS   = 1000
THREADS = 8
TYPE    = sse_unaligned

CC      = gcc # clang
CFLAGS  = -O3 -ffast-math -ffp-contract=fast #-march=native

$(AVX):  CFLAGS += -mavx
$(AVX2): CFLAGS += -mavx2
$(FMA):  CFLAGS += -mfma
$(SSE):  CFLAGS += -msse

default: compile run

%.elf: $(OBJS) $(AVX) $(SSE)
	$(CC) -pthread -o $@ $^ -lm

.INTERMEDIATE: $(OBJS) $(AVX) $(SSE)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<



compile: $(TARGET)

run: compile
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE)

bench: compile
	python bench.py

ascii: compile
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE) --ascii=1

visualize: compile
	./$(TARGET) --timesteps=$(STEPS) --width=$(WIDTH) --height=$(HEIGHT) --pulseX=$(PULSEX) --pulseY=$(PULSEY) --threads=$(THREADS) --kernel=$(TYPE) --output
	./ximage_x86 n1=$(HEIGHT) n2=$(WIDTH) hbox=$(HEIGHT) wbox=$(WIDTH) title=visualizer < output.bin

objdump: compile
	objdump -dS $(TARGET) | less

clean:
	rm $(TARGET) *.o *.bin -rf
