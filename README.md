# Septima library documentation

## Introduction

### Overview
This is a C++ library for investigating tonal relations between seventh chords. It can
* generate elementary transitions between seventh chords,
* create chord networks,
* find optimal voicings for sequences of seventh chord symbols.

### Dependencies
1. GNU Linear Programming Kit (GLPK)
2. GNU Scientific Library (GSL)

### Installation
To compile the library, type "make".

## Command-line interface

After a successful compilation, executable `septima` will appear in the installation directory.

### Usage

On Linux terminal, the executable is called like this:

`./septima <task> [<option(s)>] CHORDS or FILE`

#### Tasks
- `-h`, `--help` &mdash; Show this help message.
- `-t`, `--transition` &mdash; Generate transitions between two seventh chords.
- `-tc`, `--transition-classes` &mdash; Generate all structural classes of transitions between seventh chords.
- `-cg`, `--chord-graph` &mdash; Create chord graph from a set of chords.
- `-v`, `--voicing` &mdash; Find an optimal voicing for the given chord sequence.
- `-av`, `--all-voicings` &mdash; Find all optimal voicings for the given chord sequence.

#### Options
- `-c`, `--class` &mdash; Specify upper bound for voice-leading infinity norm (default: 7).
- `-dg`, `--degree` &mdash; Specify degree of elementary transitions (default: unset).
- `-aa`, `--allow-augmented` &mdash; Allow augmented realizations (by default, German sixths and Tristan chords are disabled).
- `-d`, `--domain` &mdash; Specify domain on the line of fifths (default: {−15,−14,...,15}, which corresponds to notes from Gbb to A##).
- `-z`, `--tonal-center` &mdash; Specify tonal center on the line of fifths (default: 0, which corresponds to the note D).
- `-lf`, `--label-format` &mdash; Specify format for chord graph labels. Choices are *symbol*, *number*, and *latex* (by default: *symbol*).
- `-p`, `--preparation` &mdash; Specify preparation scheme for elementary transitions. Choices are *none*, *generic*, and *acoustic* (by default: *none*).
- `-w`, `--weights` &mdash; Specify weight parameters for voicing algorithm. Three nonnegative floating-point values are required: tonal-center proximity weight *w1*, voice-leading complexity weight *w2*, and penalty *w3* for augmented sixths (by default, *w1* = 1.0, *w2* = 1.75, and *w3* = 0.25),
- `-ly`, `--lilypond` &mdash; Output transitions and voicings in Lilypond code.
- `-q`, `--quiet` &mdash; Suppress messages.
