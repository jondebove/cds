.POSIX:

CFLAGS = -O2 -Wall -Wextra #-DNDEBUG

OBJS = darray.o dstring.o htable.o
TESTS = darray_test dstring_test htable_test

.PHONY: all clean

all: $(TESTS)

darray_test: darray_test.c darray.o
dstring_test: dstring_test.c dstring.o
htable_test: htable_test.c htable.o

darray.o: darray.c darray.h
dstring.o: dstring.c dstring.h
htable.o: htable.c htable.h

clean:
	-rm -f $(OBJS) $(TESTS)
