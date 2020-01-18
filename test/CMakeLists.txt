#
# component tests
#
add_test(NAME PLAIN_NAIIV_1_Thread COMMAND ${TARGETELF} --timesteps=1000 --width=1000 --height=516 --pulseX=600 --pulseY=70 --threads=1 --kernel=plain_naiiv --output=seismic_ref.bin)
add_test(NAME PLAIN_OPT_8_Threads COMMAND ${TARGETELF} --timesteps=1000 --width=1000 --height=516 --pulseX=600 --pulseY=70 --threads=8 --kernel=plain_opt --output=seismic_chk.bin)
