CC=g++
CFLAGS=-I.
DEPS=chord.h chordgraph.h matrix.h realization.h tone.h transition.h
OBJ=main.o chord.o chordgraph.o matrix.o realization.o tone.o transition.o 
LIBS=-lglpk -lm
PROGRAMS=septima

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROGRAMS): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ $(PROGRAMS) 
