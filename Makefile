.POSIX:

CFLAGS = -O2 -Wall -Wextra #-DNDEBUG

OBJS = darray.o htable.o
TESTS = darray_test htable_test

.PHONY: all clean

all: $(TESTS)

darray_test: darray_test.c darray.o
htable_test: htable_test.c htable.o

darray.o: darray.c darray.h
htable.o: htable.c htable.h

clean:
	-rm -f $(OBJS) $(TESTS)
