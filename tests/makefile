CFLAGS=-std=c99 -g

all: buftest.out errtest.out rtabtest.out mylangtest.out


#buffer test
soui.o: ../soui.c ../soui.h
	gcc $(CFLAGS) $< -o $@ -c

qoui.o: ../qoui.c ../qoui.h
	gcc $(CFLAGS) $< -o $@ -c

buffer.o: ../buffer.c ../buffer.h ../soui.h ../qoui.h ../boolean.h
	gcc $(CFLAGS) $< -o $@ -c

buftest.out: buftest.c buffer.o qoui.o soui.o restab.o itrerror.o
	gcc $(CFLAGS) $^ -o $@

# error test
itrerror.o: ../itrerror.c ../itrerror.h
	gcc $(CFLAGS) $< -o $@ -c

errtest.out: errtest.c itrerror.o restab.o
	gcc $(CFLAGS) $^ -o $@ 

# restab test
restab.o: ../restab.c ../restab.h
	gcc $(CFLAGS) $< -o $@ -c

rtabtest.out: rtabtest.c restab.o itrerror.o
	gcc $(CFLAGS) $^ -o $@

# my lang
mylangtest.out: mylang.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -f *.o *.out vgcore.*
