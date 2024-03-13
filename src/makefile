# Compiler version and flags
CC=gcc
CFLAGS= -g -std=gnu99 -Wall -pedantic -I $(IDIR)


PROGRAM=start
TESTER=as1_testbench

# Directories
ODIR=./obj
LDIR =../lib
IDIR =../include

# List all .h and .c files
_DEPS = bank.h client.h constants.h socket.h
_OBJ = bank.o socket.o
_OBJ_CLIENT = client.o socket.o
_OBJ_BENCH = as2_testbench.o

TFLAGS=-std=gnu99 -g -Wall -pedantic

# Create directories for the header files
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# Create the directory lists for object files
# using the object file list and object dir
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
OBJ_CLIENT = $(patsubst %,$(ODIR)/%,$(_OBJ_CLIENT))
OBJ_BENCH = $(patsubst %,$(ODIR)/%,$(_OBJ_BENCH))

# Create dependencies between the .o, .c and .h files
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile everything.
.PHONY: all
all: bank client bench

# Compile command for bench
bench: $(OBJ_BENCH)
	$(CC) -o bench $^ $(TFLAGS)

# Compile command for bank
bank: $(OBJ)
	$(CC) -o bank $^ $(CFLAGS) $(LIBS)

# Compile command for client
client: $(OBJ_CLIENT)
	$(CC) -o client $^ $(CFLAGS) $(LIBS)

# clean
.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ bank client bench
