# Copyright 2017 - , Dr.-Ing. Patrick Siegl
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.1)

project(seismic-rtm VERSION 0.0.1 LANGUAGES C)

# This project can use C11, but will gracefully decay down to C89.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS ON)

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Detecting architecture - ${ARCHITECTURE}" )

find_package( Threads REQUIRED )

include_directories(src src/barrier src/kernel)

SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -ffast-math -ffp-contract=fast -fprefetch-loop-arrays")

file(GLOB SOURCES src/*.c)
list(APPEND SOURCES src/kernel/kernel_plain.c)

if( ${ARCHITECTURE} STREQUAL "x86_64" )

  list(APPEND SOURCES src/kernel/kernel_sse.c
                      src/kernel/kernel_sse_fma.c
                      src/kernel/kernel_avx.c
                      src/kernel/kernel_avx_fma.c
                      src/kernel/kernel_sse_avx_fma_partial_aligned.c
                      src/kernel/kernel_avx2.c
                      src/kernel/kernel_avx2_fma.c)
  # https://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/i386-and-x86_002d64-Options.html
  set_source_files_properties( src/kernel/kernel_sse_fma.c  PROPERTIES COMPILE_FLAGS "-mfma" )
  set_source_files_properties( src/kernel/kernel_avx.c      PROPERTIES COMPILE_FLAGS "-mavx" )
  set_source_files_properties( src/kernel/kernel_sse_avx_fma_partial_aligned.c
                               src/kernel/kernel_avx_fma.c  PROPERTIES COMPILE_FLAGS "-mavx -mfma" )
  set_source_files_properties( src/kernel/kernel_avx2.c     PROPERTIES COMPILE_FLAGS "-mavx2" )
  set_source_files_properties( src/kernel/kernel_avx2_fma.c PROPERTIES COMPILE_FLAGS "-mavx2 -mfma" )

elseif( ${ARCHITECTURE} STREQUAL "armv6l"
        OR ${ARCHITECTURE} STREQUAL "armv7l"
        OR ${ARCHITECTURE} STREQUAL "aarch64" )

  list(APPEND SOURCES src/kernel/kernel_ARM_NEON.c)
  set_source_files_properties( src/kernel/kernel_ARM_NEON.c PROPERTIES COMPILE_FLAGS "-mfpu=neon" )

elseif( ${ARCHITECTURE} STREQUAL "ppc"
        OR ${ARCHITECTURE} STREQUAL "ppcle"
        OR ${ARCHITECTURE} STREQUAL "ppc64"
        OR ${ARCHITECTURE} STREQUAL "ppc64le" )

  list(APPEND SOURCES src/kernel/kernel_vmx.c
                      src/kernel/kernel_vsx.c)
  set_source_files_properties( src/kernel/kernel_vmx.c      PROPERTIES COMPILE_FLAGS "-maltivec -mabi=altivec" )
  set_source_files_properties( src/kernel/kernel_vsx.c      PROPERTIES COMPILE_FLAGS "-maltivec -mabi=altivec -mvsx" )

endif()

add_executable(seismic-rtm.elf ${SOURCES})
target_link_libraries (seismic-rtm.elf Threads::Threads m)
