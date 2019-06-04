# Copyright 2017 - , Dr.-Ing. Patrick Siegl
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.1)

project(seismic-rtm VERSION 0.0.1 LANGUAGES C)

# This project can use C11, but will gracefully decay down to C89.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS ON)

find_package(Threads REQUIRED)


################## cpu_features
configure_file(CMakeLists.txt.in cpu_features-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cpu_features-download )
if(result)
  message(FATAL_ERROR "CMake step for cpu_features failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cpu_features-download )
if(result)
  message(FATAL_ERROR "Build step for cpu_features failed: ${result}")
endif()

# Add cpu_features directly to our build.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/cpu_features-src
                 ${CMAKE_CURRENT_BINARY_DIR}/cpu_features-build
                 EXCLUDE_FROM_ALL)

message(STATUS "Detecting architecture - ${CMAKE_SYSTEM_PROCESSOR}")
##################


include_directories(src src/barrier src/kernel)

SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -ffast-math -ffp-contract=fast -fprefetch-loop-arrays")

file(GLOB SOURCES src/*.c)
list(APPEND SOURCES src/kernel/kernel_plain.c)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(^i.86$)")

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

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm"
       OR CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")

  list(APPEND SOURCES src/kernel/kernel_ARM_NEON.c)
  set_source_files_properties( src/kernel/kernel_ARM_NEON.c PROPERTIES COMPILE_FLAGS "-mfpu=neon" )

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")

  list(APPEND SOURCES src/kernel/kernel_vmx.c
                      src/kernel/kernel_vsx.c)
  set_source_files_properties( src/kernel/kernel_vmx.c      PROPERTIES COMPILE_FLAGS "-maltivec -mabi=altivec" )
  set_source_files_properties( src/kernel/kernel_vsx.c      PROPERTIES COMPILE_FLAGS "-maltivec -mabi=altivec -mvsx" )

endif()

add_executable(seismic-rtm.elf ${SOURCES})
target_link_libraries (seismic-rtm.elf Threads::Threads m cpu_features)
