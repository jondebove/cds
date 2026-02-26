.POSIX:

CFLAGS = -Wall -Wextra #-DNDEBUG

OBJS = darray.o dstring.o heap.o htable.o
TESTS = darray_test dstring_test heap_test htable_test

.PHONY: all clean

all: $(TESTS)

darray_test: darray_test.c darray.o
dstring_test: dstring_test.c dstring.o
heap_test: heap_test.c heap.o
htable_test: htable_test.c htable.o

darray.o: darray.c darray.h
dstring.o: dstring.c dstring.h
heap.o: heap.c heap.h
htable.o: htable.c htable.h

clean:
	-rm -f $(OBJS) $(TESTS)
