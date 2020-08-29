/* tone.h
 *
 * Copyright (c) 2020  Luka Marohnić
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
std::ostream& operator <<(std::ostream &os, const std::vector<Tone> &tv);
std::ostream& operator <<(std::ostream &os, const std::set<Tone> &ts);

#endif // TONE_H
