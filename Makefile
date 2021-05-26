CC:=$(CROSS_COMPILE)gcc

EDCFLAGS:= -O2 -Wall -std=gnu11 -I ./ $(CFLAGS)
EDLDFLAGS:= -lpthread -lm $(LDFLAGS)

COBJS := uhf_modem.o

all: gs_test sh_test

sh_test: sh_test.o $(COBJS) $(CPPOBJS)
	$(CC) -o $@.out $< $(COBJS) $(EDLDFLAGS)
gs_test: gs_test.o $(COBJS) $(CPPOBJS)
	$(CC) -o $@.out $< $(COBJS) $(EDLDFLAGS)

objfiles: $(COBJS) $(CPPOBJS)

%.o: %.c
	$(CC) $(EDCFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -vrf *.o
	rm -vrf *.out