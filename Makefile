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

TARGET          := seismic.elf
OBJS            := config.o visualize.o pthreading.o 
OPENMP_MAIN     := main.o
OPENMP          := openmp.o
PLAIN           := kernel_plain.o
SSE             := kernel_sse.o
SSE_FMA         := kernel_sse_fma.o
SSE_AVX         := kernel_sse_avx_partial_aligned.o
SSE_AVX_FMA     := kernel_sse_avx_fma_partial_aligned.o
AVX             := kernel_avx.o
AVX_FMA         := kernel_avx_fma.o
AVX2            := kernel_avx2.o
AVX2_FMA        := kernel_avx2_fma.o

CC              = gcc # clang
CFLAGS          = -ffast-math -ffp-contract=fast -Ofast #-march=native

$(PLAIN):       CFLAGS += -mno-sse -mno-fma -mno-avx -mno-avx2 # plain_opt version gains a lot with -msse
$(SSE):         CFLAGS += -msse    -mno-fma -mno-avx -mno-avx2
$(SSE_FMA):     CFLAGS += -msse    -mfma    -mavx    -mno-avx2 # fma only allowed with avx
$(SSE_AVX):     CFLAGS += -msse    -mno-fma -mavx    -mno-avx2
$(SSE_AVX_FMA): CFLAGS += -msse    -mfma    -mavx    -mno-avx2
$(AVX):         CFLAGS += -msse    -mno-fma -mavx    -mno-avx2
$(AVX_FMA):     CFLAGS += -msse    -mfma    -mavx    -mno-avx2
$(AVX2):        CFLAGS += -msse    -mno-fma -mavx    -mavx2
$(AVX2_FMA):    CFLAGS += -msse    -mfma    -mavx    -mavx2
$(OPENMP_MAIN): CFLAGS += -fopenmp
$(OPENMP):      CFLAGS += -mno-sse -mno-fma -mno-avx -mno-avx2 -fopenmp

default: compile run

%.elf: $(OBJS) $(PLAIN) $(SSE) $(SSE_FMA) $(SSE_AVX) $(SSE_AVX_FMA) $(AVX) $(AVX_FMA) $(AVX2) $(AVX2_FMA) $(OPENMP) $(OPENMP_MAIN)
	$(CC) -fopenmp -pthread -o $@ $^ -lm

.INTERMEDIATE: $(OBJS) $(PLAIN) $(SSE) $(SSE_FMA) $(SSE_AVX) $(SSE_AVX_FMA) $(AVX) $(AVX_FMA) $(AVX2) $(AVX2_FMA) $(OPENMP) $(OPENMP_MAIN)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

compile: $(TARGET)



WIDTH   = 1000
HEIGHT  = 516
PULSEX  = 600
PULSEY  = 70
STEPS   = 1000
THREADS = 8
TYPE    = sse_unaligned

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
