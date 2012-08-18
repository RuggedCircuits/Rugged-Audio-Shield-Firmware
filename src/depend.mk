adc.o: adc.c config.h rec.h ff.h integer.h ffconf.h functable.h timer.h \
 sio.h utils.h state.h adc.h
bootloader.o: bootloader.c config.h bootloader.h
buffers.o: buffers.c buffers.h config.h
clocks.o: clocks.c config.h main.h utils.h clocks.h
dac.o: dac.c config.h rec.h ff.h integer.h ffconf.h functable.h timer.h \
 sio.h utils.h dac.h
dma.o: dma.c config.h buffers.h state.h dac.h play.h ff.h integer.h \
 ffconf.h functable.h rec.h dma.h
fail.o: fail.c config.H fail.h
ff.o: ff.c config.h fail.h diskio.h integer.h functable.h ff.h ffconf.h
i2c.o: i2c.c config.h timer.h i2c.h
main.o: main.c sio.h utils.h timer.h config.h clocks.h adc.h rec.h ff.h \
 integer.h ffconf.h functable.h dac.h buffers.h state.h play.h \
 spi_C_slave.h i2c.h diskio.h fail.h printf.h
pass.o: pass.c config.h dma.h state.h buffers.h rec.h ff.h integer.h \
 ffconf.h functable.h adc.h rateclock.h i2c.h play.h pass.h
play.o: play.c config.h ff.h integer.h ffconf.h functable.h adc.h rec.h \
 sio.h utils.h buffers.h state.h rateclock.h play.h dma.h i2c.h wavread.h
printf.o: printf.c config.h printf.h sio.h
rateclock.o: rateclock.c config.h rateclock.h
rec.o: rec.c config.h ff.h integer.h ffconf.h functable.h adc.h rec.h \
 buffers.h state.h wavread.h wavwrite.h dma.h rateclock.h fail.h
sio.o: sio.c config.h sio.h
spi_C_slave.o: spi_C_slave.c config.h i2c.h version.c sio.h play.h ff.h \
 integer.h ffconf.h functable.h rec.h fail.h state.h spi_C_slave.h adc.h \
 pass.h bootloader.h wavwrite.h
state.o: state.c config.h state.h
timer.o: timer.c timer.h config.h diskio.h integer.h functable.h
utils.o: utils.c sio.h utils.h
version.o: version.c
wavread.o: wavread.c config.h buffers.h wavread.h ff.h integer.h ffconf.h \
 functable.h fail.h
wavwrite.o: wavwrite.c config.h buffers.h wavread.h ff.h integer.h \
 ffconf.h functable.h wavwrite.h fail.h
