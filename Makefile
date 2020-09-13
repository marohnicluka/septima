CC=g++
CFLAGS=-I. -Wall
LIBDIR=lib
SRCDIR=src
BUILDDIR=obj
PREFIX=/usr/local
MKDIR_P=mkdir -p
SRC=chord.cpp chordgraph.cpp matrix.cpp realization.cpp tone.cpp transition.cpp transitionnetwork.cpp digraph.cpp domain.cpp
OBJ=$(SRC:%.cpp=$(BUILDDIR)/%.o)
DEPS=$(SRC:%.cpp=$(SRCDIR)/%.h)
LIBS=-lglpk -lm -lgsl -lgslcblas
PROGRAM=septima

.PHONY: all dirs clean install uninstall $(PROGRAM)

all: dirs $(PROGRAM)

dirs:
	$(MKDIR_P) $(BUILDDIR)
	$(MKDIR_P) $(LIBDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) -fPIC -c -o $@ $< $(CFLAGS)
	
$(PROGRAM): $(OBJ)
	$(CC) -c -o $@.o $@.cpp $(CFLAGS)
	$(CC) -o $@ $@.o $^ $(CFLAGS) $(LIBS)
	$(CC) -shared -o $(LIBDIR)/lib$@.so $^ $(LIBS)

clean:
	rm -f *.o *~ $(PROGRAM)
	rm -rf $(BUILDDIR) $(LIBDIR)

install: $(LIBDIR)/lib$(PROGRAM).so
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/bin
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/include/$(PROGRAM)
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/lib
	cp $^ $(DESTDIR)$(PREFIX)/lib/.
	cp $(PROGRAM) $(DESTDIR)$(PREFIX)/bin/.
	cp $(DEPS) $(DESTDIR)$(PREFIX)/include/$(PROGRAM)/.

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PROGRAM)
	rm -f $(DESTDIR)$(PREFIX)/lib/lib$(PROGRAM).so
	rm -rf $(DESTDIR)$(PREFIX)/include/$(PROGRAM)
