/* transition.h
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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "realization.h"
#include "domain.h"
#include <algorithm>

enum PreparationScheme {
    NONE = 0,
    ACOUSTIC = 1,
    ACOUSTIC_NO_DOMINANT = 2,
    GENERIC = 3
};

class Transition {

    Realization _first;
    Realization _second;

    struct notated_chord_compare {
        int center;
        bool operator ()(const std::vector<int> &nc1, std::vector<int> &nc2) const {
            double d1 = 0, d2 = 0;
            std::vector<int>::const_iterator it;
            for (it = nc1.begin() + 1; it != nc1.end(); ++it) {
                d1 += (*it - center) * (*it - center);
            }
            for (it = nc2.begin() + 1; it != nc2.end(); ++it) {
                d2 += (*it - center) * (*it - center);
            }
            d1 /= double(nc1.size());
            d2 /= double(nc2.size());
            return d1 < d2;
        }
        notated_chord_compare(int c = 71) { center = c; }
    };

public:
    Transition() { }
    Transition(const Realization &a, const Realization &b);
    Transition(const Transition &other);
    Transition& operator =(const Transition &other);
    bool operator ==(const Transition &other) const;
    bool operator !=(const Transition &other) const;
    bool operator <(const Transition &other) const;

    void set_first(const Realization &r);
    /* sets the first chord */

    void set_second(const Realization &r);
    /* sets the second chord */

    const Realization &first() const;
    /* gets the first chord */

    const Realization &second() const;
    /* gets the second chord */

    double MAD(int z) const;
    /* returns the distance between this transition and the point z on the line of fifths */

    double lof_distance(int z, bool maximum = true) const;
    /* returns the maximum distance between a tone in this transition and z on the line of fifths */

    int diameter() const;
    /* return the diameter of the pattern on the line of fifths */

    std::set<Tone> tone_set() const;
    /* returns the union of sets of tones in both realizations */

    bool is_closer_than(const Transition &other, int z) const;
    /* returns true iff this transition is closer to z than the other on the line of fifths */

    bool glue(const Realization &pred, int &mc, int &tcn, std::vector<int> &f, int k = 7) const;
    /* Construct a voice leading from pred to the first chord in this, which have to be enharmonically equal,
     * returns true on success
     *  - mc is the number of mandatory cues, f the voice leading
     *  - tcn is the taxicab norm of voice leaading from pred to the second realization
     *  - f is the voice leading from pred to the first realization (enharmonic mapping)
     *  - k is the class index */

    Transition structural_inversion() const;
    /* returns the structural inversion of this transition */

    Transition retrograde() const;
    /* returns the retrograde transition */

    bool is_enharmonically_equal(const Transition &other) const;
    /* returns true iff this transition is enharmonically equal to the other */

    bool is_congruent(const Transition &other) const;
    /* returns true iff this transition is structurally equal to the other */

    bool is_structurally_equal(const Transition &other, bool enharm = false) const;
    /* returns true iff this transition is congruent to the other */

    bool is_equivalent_up_to_transposition_and_rotation(const Transition &other) const;
    /* returns true iff this transition is a transposition and/or rotation of the other */

    bool is_smooth() const;
    /* returns true iff each voice moves by at most two semitones */

    bool is_efficient() const;
    /* returns true iff the transition has the minimal voice-leading shift */

    std::string to_string() const;
    /* returns a string representation */

    std::string to_lily(int mp = 71, int prep = 0, bool ch = false) const;
    /* returns a lilypond representation with tones close to midi pitch mp
     *  - parallel generic fifths are avoided
     *  - preparation is indicated for prep = 1, 2 (set prep = 0 to ignore preparation)
     *  - chord types are indicated above the notation iff ch = true
     */

    double lof_spread() const;
    /* returns the standard deviation of the union of realizations X and Y in this transition */

    double vl_lof_spread() const;
    /* returns the standard deviation of the set {(v(x)+x)/2 : x in X}, given this = X ->v Y */

    int augmented_count(bool tristan = false) const;
    /* returns the number of augmented-sixth realizations in this transition */

    int vl_shift() const;
    /* returns the total voice-leading shift (Kochavi 2008) */

    int directional_vl_shift() const;
    /* returns the total voice-leading shift with directions (Kochavi 2008) */

    int degree() const;
    /* returns the degree of this progression (smallest M such that it is of class M) */

    int common_pc_count() const;
    /* returns the number of common pitch classes in both realizations */

    bool acts_identically_on_pc_intersection() const;
    /* returns true iff the voice leading acts identically on the intersection of the corresponding pcsets */

    bool is_prepared_generic() const;
    /* returns true iff the generic seventh in the second realization is prepared in the first */

    void transpose(int d);
    /* shift both realizations for d steps on the line of fifths */

    static std::set<Transition> elementary_transitions(const Chord &c1, const Chord &c2, int k, const Domain &dom, PreparationScheme p, bool aug);
    /* returns the list of parsimonious transitions from c1 to c2 in the domain dom on the line-of-fifths
     *  - k is the class index
     *  - p is the preparation scheme
     *  - augmented-sixth realizations are used iff aug = true
     */

    static std::vector<Transition> elementary_classes(const Chord &c1, const Chord &c2, int k, PreparationScheme p, int z, bool aug);
    /* returns the list of transitions representing the structural equivalence classes of elementary transitions
     * of class k from c to d (representatives are chosen near z on the line of fifths) */

    static std::vector<Transition> elementary_types(const std::vector<Chord> &chords, int k, PreparationScheme p, int z, bool aug);
    /* returns the set of types of elementary transitions of class k, enharmonic classes are simplifed */

    static std::set<std::vector<Transition> > enharmonic_classes(const std::vector<Transition> &st);
    /* returns the set of enharmonic classes of transitions in st */

    static void simplify_enharmonic_class(std::vector<Transition> &st);
    /* simplifies the class st of enharmonically equal transitions by
     * respelling augmented sixths and reducing the voice-leading infinity norm
     */

    static const int sym4[][4];
    static const char* chord_type_names[];
};

std::ostream& operator <<(std::ostream &os, const Transition &t);
std::ostream& operator <<(std::ostream &os, const std::vector<Transition> &tv);

#endif // TRANSITION_H
