CC=g++
CFLAGS=-I.
DEPS=lib/chord.h lib/chordgraph.h lib/matrix.h lib/realization.h lib/tone.h lib/transition.h lib/transitionnetwork.h lib/digraph.h lib/domain.h
OBJ=septima.o lib/chord.o lib/chordgraph.o lib/matrix.o lib/realization.o lib/tone.o lib/transition.o lib/transitionnetwork.o lib/digraph.o lib/domain.o
LIBS=-lglpk -lm -lgsl -lgslcblas
PROGRAMS=septima

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROGRAMS): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o lib/*.o *~ $(PROGRAMS) 
