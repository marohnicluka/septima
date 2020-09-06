#Septima

##Introduction

###Overview
This is a C++ library for generating tonal realizations of seventh chords and chord graphs.

###Dependencies
1. GNU Linear Programming Kit (GLPK)
2. GNU Scientific Library (GSL)

###Installation
To compile, type "make".

##Command-line Interface

After successful compilation, executable `septima` will appear in the installation directory.

###Usage

On Linux, the executable is called from terminal like this:

`./septima <task> [<option(s)>] CHORDS or FILE`

####Tasks
- `-h`, `--help` --- Show this help message
- `-t`, `--transition` --- Generate transitions between two seventh chords
