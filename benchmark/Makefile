SYSROOT=$(HOME)/ld-exploration/obj/home/gp472/ld-exploration/freebsd-src/amd64.amd64/tmp

CFLAGS=-Wall -g --sysroot=$(SYSROOT) -DCOMPILING_ON_ZENO \
       -DTEST_LIBRARIES_ROOT=\"$(HOME)/ld-exploration/obj/home/gp472/ld-exploration/freebsd-src/amd64.amd64/libexec/rtld-elf/tests\"

all: benchmark-dlopen benchmark-dlopen_sandbox

benchmark-dlopen.o: dlopen-benchmark.c
	cc -c -o ${.TARGET} ${CFLAGS} -DBENCHMARK_DLOPEN dlopen-benchmark.c -dynamic

benchmark-dlopen_sandbox.o: dlopen-benchmark.c
	cc -c -o ${.TARGET} ${CFLAGS} -DBENCHMARK_DLOPEN_SANDBOX dlopen-benchmark.c -dynamic

benchmark-dlopen: benchmark-dlopen.o
	c++ -o ${.TARGET} ${.ALLSRC} -lpmc -dynamic -lxo -lutil

benchmark-dlopen_sandbox: benchmark-dlopen_sandbox.o
	c++ -o ${.TARGET} ${.ALLSRC} $(SYSROOT)/lib/libc.so.7 -lpmc -dynamic -lxo -lutil

clean:
	rm *.o benchmark-dlopen benchmark-dlopen_sandbox

