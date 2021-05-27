CC:=$(CROSS_COMPILE)gcc

EDCFLAGS:= -O2 -Wall -std=gnu11 -I ./ $(CFLAGS)
EDLDFLAGS:= -lpthread -lm $(LDFLAGS)

COBJS := uhf_modem.o

all: gs_test sh_test

sh_test: test/sh_test.o $(COBJS)
	$(CC) -o $@.out $< $(COBJS) $(EDLDFLAGS)

gs_test: test/gs_test.o $(COBJS)
	$(CC) -o $@.out $< $(COBJS) $(EDLDFLAGS)

objfiles: $(COBJS)

%.o: %.c
	$(CC) $(EDCFLAGS) -o $@ -c $<

.PHONY: clean doc

doc:
	doxygen .doxyconfig

clean:
	rm -vf *.o
	rm -vf test/*.o
	rm -vf *.out

spotless: clean
	rm -rf doc/