SRCS		= buffer.c lexer.c restab.c itrerror.c qoui.c soui.c symtab.c       \
			  ilist.c exprtree.c prectab.c kwrecognizer.c parser.c soexpitems.c \
			  cmptcheck.c inter.c fcallstack.c ifj16funcs.c

OBJS		= $(SRCS:.c=.o)
CFLAGS	= -std=c11 -pedantic -Wextra -Wall -g

all: ifj16.java

%.o: %.c %.h
	gcc $(CFLAGS) $< -o $@ -c

ifj16.java: ifj16.c $(OBJS)
	gcc $(CFLAGS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCS)
	gcc -MM $^ > $@

clean:
	rm -f ifj16.java *.o *.out vgcore.*