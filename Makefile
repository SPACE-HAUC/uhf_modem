CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++

EDCFLAGS:= -O2 -Wall -std=gnu11 -I include/ $(CFLAGS) $(CXXFLAGS)
EDLDFLAGS:= -lpthread -lm $(LDFLAGS)

HOSTNAME:=$(shell hostname) # used for hostname specific things
# ifeq ($(HOSTNAME),spacehauc)
#
# endif
SRC_DIR := src
SRC_C := $(wildcard $(SRC_DIR)/*.c)
COBJS = $(SRC_C:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)
SRC_CPP := $(wildcard $(SRC_DIR)/*.cpp)
CPPOBJS = $(SRC_CPP:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)

TARGET := test.out

all: gs_test sh_test

sh_test: sh_test.o $(COBJS) $(CPPOBJS)
	$(CC) $< $(COBJS) $(EDLDFLAGS)
gs_test: gs_test.o $(COBJS) $(CPPOBJS)
	$(CC) $< $(COBJS) $(EDLDFLAGS)

objfiles: $(COBJS) $(CPPOBJS)

%.o: %.c
	$(CC) $(EDCFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(EDCFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf $(SRC_DIR)/*.o
	rm -rf ./*.o
	rm -rf $(TARGET)