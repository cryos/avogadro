LELEMENTS = msym msym_error context symmetry symmetrize linalg point_group character_table geometry orbital\
		permutation symop equivalence_set elements

SRC = example.c
OBJDIR = ../obj
BINDIR = ../bin
OBJS = $(OBJDIR)/example.o
LSRC = $(foreach I,$(LELEMENTS),../src/$I.c)
LOBJS = $(foreach I,$(LELEMENTS),$(OBJDIR)/$I.o)



CC = gcc
EXE = $(BINDIR)/msym_example
CFLAGS = -g -O2 -flto -std=c99 -pedantic -Wall
INC = ../src
LIBS = -lm

all:    $(OBJDIR) $(BINDIR) $(OBJS) $(LOBJS)
	$(CC) -o $(EXE) $(CFLAGS) $(OBJS) $(LOBJS) $(LIBS)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

$(BINDIR) :
	mkdir -p $(BINDIR)

../obj/example.o: example.c
	$(CC) ${CFLAGS} -I $(INC) -o $@ -c $<

../obj/%.o: ../src/%.c
	$(CC) ${CFLAGS} -I $(INC) -o $@ -c $<

clean:  
	$(RM) -rf $(OBJDIR) $(BINDIR)

