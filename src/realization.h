/* realization.h
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

#ifndef REALIZATION_H
#define REALIZATION_H

#include "tone.h"
#include "chord.h"
#include "domain.h"
#include "digraph.h"
#include <vector>
#include <set>

class Realization {

    Tone _tones[4];
    Chord _chord;

    static const int lof_structure[][4];

public:
    Realization() { }
    Realization(const Chord &c); // constructs a realization for the chord c
    Realization(const Realization &other);
    ~Realization() { }
    Realization& operator =(const Realization &other);
    bool operator ==(const Realization &other) const;
    bool operator !=(const Realization &other) const;
    bool operator <(const Realization &other) const;

    bool is_enharmonically_equal(const Realization &other) const;
    /* is this realization enharmonically equivalent to the other? */

    bool is_augmented_sixth(bool tristan = false) const;
    /* is this an augmented chord (German sixth or Tristan chord)? */

    int generic_root_voice() const;
    /* returns the voice containing the generic root of the chord */

    int generic_seventh_voice() const;
    /* returns the voice containing the generic seventh of the chord */

    int acoustic_seventh_voice() const;
    /* returns the voice containing the acoustic seventh or -1 if there is none */

    int type() const;
    /* returns the type of the realized chord */

    const Chord& chord() const;
    /* returns the chord */

    void arrange(const std::vector<int> &perm);
    /* rearranges the tones */

    void transpose(int d);
    /* shift all tones for d steps on the line of fifths */

    const Tone& tone(int i) const;
    Tone& tone(int i);
    /* returns the i-th tone of the realization */

    std::set<Tone> tone_set() const;
    /* returns the set of tones */

    Realization structural_inverse() const;
    /* returns the structural inverse of this realization */

    double lof_point_distance(int z) const;
    /* return the distance between this realization and the point z on the line of fifths */

    bool check_fifths() const;
    /* returns true iff each generic fifth is either perfect or diminished */

    std::string to_string() const;
    /* returns a string representation */

    static std::set<ivector> lof_patterns(const Chord &c, int &tot, int &ton, const Domain &dom);
    /* returns the set of realization patterns for c in domain dom on the line of fifths
     *  - tot is the total number of realizations
     *  - ton is the number of realizations for which check_fifths returns true (if diam(dom)<=30,
     *    then ton is the number of tonal realizations)
     */

    static std::vector<Realization> tonal_realizations(const Chord &c, const Domain &dom, bool aug);
    /* returns the list of all tonal realizations of the chord c in domain dom on the line of fifths
     *  - augmented realizations are included iff aug = true
     */
};

std::ostream& operator <<(std::ostream &os, const Realization &r);
/* write realization r to the output stream os */

std::ostream& operator <<(std::ostream &os, const std::vector<Realization> &rv);
/* write comma-separated list of reallizations rv to the output stream os */

#endif // REALIZATION_H
