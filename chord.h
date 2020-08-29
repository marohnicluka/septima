/* chord.h
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
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHORD_H
#define CHORD_H

#include <string>
#include <set>
#include <vector>
#include <ostream>

/* types of seventh chord realizations */
enum ChordType {
    DOMINANT        = 0,
    HALF_DIMINISHED = 1,
    MINOR           = 2,
    MAJOR           = 3,
    DIMINISHED      = 4,
    GERMAN_SIXTH    = 5,
    TRISTAN         = 6
};

typedef std::pair<int,int> ipair;

class Chord {

    int _root;
    int _type;
    static void set_differences(std::set<int> &pc1, std::set<int> &pc2, std::vector<int> &X, std::vector<int> &Y);

public:
    Chord();
    Chord(int r, int t);
    Chord(const Chord &other);
    ~Chord() { }
    Chord& operator =(const Chord &other);
    bool operator ==(const Chord &other) const;
    bool operator !=(const Chord &other) const;
    static const int structure[][3];
    static const char *type_string[];

    int root() const;
    /* returns the root pitch class of this chord */

    int type() const;
    /* returns the type of this chord (0--4) */

    void set_root(int r);
    /* sets the root pitch */

    void set_type(int t);
    /* sets the type (0--4) */

    int third() const;
    /* returns the pitch class of the third */

    int fifth() const;
    /* returns the pitch class of the fifth */

    int seventh() const;
    /* returns the pitch class of the seventh */

    std::set<ipair> Pmn_relations(const Chord &other) const;
    /* returns the set of all P_{m,n} relations between this chord and other (Douthett & Steinbach, 1998) */

    int vl_efficiency_metric(const Chord &other) const;
    /* returns the value of voice-leading efficiency metric (Harasim et.al, 2016) */

    std::string to_string() const;
    /* returns the string representation */

    std::string to_tex() const;
    /* returns the LaTeX label (for math mode) */

    std::set<int> pitch_class_set() const;
    /* returns the integer representation */

    Chord structural_inversion() const;
    /* returns the structural inversion */
};

std::ostream& operator <<(std::ostream &os, const Chord &c);
std::ostream& operator <<(std::ostream &os, const std::vector<Chord> &cv);

#endif // CHORD_H
