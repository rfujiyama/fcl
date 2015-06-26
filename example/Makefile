CC?=gcc
CFLAGS+=-I../ -march=native -pipe -std=c11 -Wall -Werror -Wextra -Wpedantic
#CFLAGS+=-DNDEBUG -O3
CFLAGS+=-g -O0
LDFLAGS=
EXES=fcl_list_fifo fcl_list_dl
OBJS=

ifeq ($(CC), clang)
	CFLAGS+=-Wno-error-unused-command-line-argument
endif

default: $(EXES)
fcl_list_fifo: $(OBJS)
		$(CC) $(CFLAGS) $@.c $(OBJS) -o $@ $(LDFLAGS)
fcl_list_dl: $(OBJS)
		$(CC) $(CFLAGS) $@.c $(OBJS) -o $@ $(LDFLAGS)
%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(EXES)
