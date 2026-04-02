.POSIX:

CFLAGS = -Wall -Wextra #-DNDEBUG

OBJS = comb.o darray.o dstring.o fnv1a.o heap.o htable.o slice.o
TESTS = comb_test darray_test dstring_test fnv1a_test heap_test htable_test slice_test

.PHONY: all clean

all: $(TESTS)

comb_test: comb_test.c comb.o
darray_test: darray_test.c darray.o
dstring_test: dstring_test.c dstring.o
fnv1a_test: fnv1a_test.c fnv1a.o
heap_test: heap_test.c heap.o
htable_test: htable_test.c htable.o
slice_test: slice_test.c slice.o

comb.o: comb.c comb.h
darray.o: darray.c darray.h
dstring.o: dstring.c dstring.h
fnv1a.o: fnv1a.c fnv1a.h
heap.o: heap.c heap.h
htable.o: htable.c htable.h
slice.o: slice.c slice.h

clean:
	-rm -f $(OBJS) $(TESTS)
