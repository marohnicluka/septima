/* tone.h
 *
 * Copyright (c) 2020  Luka Marohnić
 *
 * This file is part of Septima.
 *
 * Septima is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Septima is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Septima.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TONE_H
#define TONE_H

#include "chord.h"
#include <string>
#include <iostream>
#include <vector>
#include <set>

class Tone {

    int _lof; // position in the line-of-fifths (0 corresponds to D)

public:
    Tone();
    Tone(int lof);
    Tone(const Tone &other);
    ~Tone() { }
    Tone& operator =(const Tone &other);
    bool operator ==(const Tone &other) const;
    bool operator !=(const Tone &other) const;
    bool operator <(const Tone &other) const;

    int lof_position() const;
    /* returns the position in the line-of-fifths */

    int note_name() const;
    /* returns the note name (0 - C, 1 - D, 2 - E, 3 - F, 4 - G, 5 - A, 6 - B) */

    int pitch_class() const;
    /* returns the pitch class (0--11) */

    int accidental() const;
    /* returns the number of accidental modifiers (sharps if positive, flats if negative, 0 means natural) */

    ipair interval(const Tone &other) const;
    /* computes the interval between this tone and the other */

    Tone structural_inversion() const;
    /* returns the structural inversion */

    void transpose(int steps);
    /* shift the tone for the given number of steps on the line of fifths */

    std::string to_string() const;
    /* returns the string representation of the tone */

    std::string to_lily() const;
    /* return Lilypond note code */

    static ipair interval_abs(const Tone &a, const Tone &b);
    /* computes the absolute interval between a and b */

    static int lof_distance(const Tone &a, const Tone &b);
    /* returns the distance between a and b on the line-of-fifths */

    static int modb(int k, int b);
    /* computes k mod b in the set {0, 1, 2, ..., b-1} */

    static int modd(int k, int b);
    /* returns the shortest distance from 0 to k mod b in the cyclic graph with vertices 0, 1, ..., b-1 */

    static int pitch_class_to_lof(int pc);
    /* return the smallest integer (by absolute value) that corresponds to pc on the line of fifths */
};

std::ostream& operator <<(std::ostream &os, const Tone &t);
/* write tone t to the output stream os */

std::ostream& operator <<(std::ostream &os, const std::vector<Tone> &tv);
/* write comma-separated list of tones tv to the output stream os */

std::ostream& operator <<(std::ostream &os, const std::set<Tone> &ts);
/* write set of tones ts to the output stream os */

#endif // TONE_H
