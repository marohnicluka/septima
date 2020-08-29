CC=g++
CFLAGS=-I.
DEPS=chord.h chordgraph.h matrix.h realization.h tone.h transition.h transitionnetwork.h digraph.h domain.h
OBJ=main.o chord.o chordgraph.o matrix.o realization.o tone.o transition.o transitionnetwork.o digraph.o domain.o
LIBS=-lglpk -lm -lgsl -lgslcblas
PROGRAMS=septima

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROGRAMS): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ $(PROGRAMS) 
