#
# component tests
#

set(DEF_SEISMIC_VALS --timesteps=1000 --width=1000 --height=516 --pulseX=600 --pulseY=70)

# Reference
add_test(NAME PLAIN COMMAND ${TARGETELF})
add_test(NAME PLAIN_NAIIV_1_Thread COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=1 --kernel=plain_naiiv --output=seismic_ref.bin)

add_test(NAME PLAIN_NAIIV_8_Thread COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=plain_naiiv --output=seismic_chk.bin)
add_test(NAME PLAIN_NAIIV_8_Thread_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)

# Check plain_opt
add_test(NAME PLAIN_OPT_8_Threads COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=plain_opt --output=seismic_chk.bin)
add_test(NAME PLAIN_OPT_8_Threads_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)

add_test(NAME PLAIN_OPT_8_Threads_CL COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=plain_opt --output=seismic_chk.bin -c)
add_test(NAME PLAIN_OPT_8_Threads_CL_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)


if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(^i.86$)")
# Check SSE
  add_test(NAME SSE_STD_1_Threads COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=1 --kernel=sse_std --output=seismic_chk.bin)
  add_test(NAME SSE_STD_1_Threads_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)

  add_test(NAME SSE_STD_8_Threads COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=sse_std --output=seismic_chk.bin)
  add_test(NAME SSE_STD_8_Threads_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)

  add_test(NAME AVX_8_Threads COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=avx_unaligned --output=seismic_chk.bin)
  add_test(NAME AVX_8_Threads_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)

  add_test(NAME AVX2_8_Threads COMMAND ${TARGETELF} ${DEF_SEISMIC_VALS} --threads=8 --kernel=avx_unaligned --output=seismic_chk.bin)
  add_test(NAME AVX2_8_Threads_BINDIFF COMMAND ${CMAKE_COMMAND} -E compare_files seismic_ref.bin seismic_chk.bin)
endif()
