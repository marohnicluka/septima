/* transitionnetwork.h
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

#ifndef TRANSITIONNETWORK_H
#define TRANSITIONNETWORK_H

#include "digraph.h"
#include "chordgraph.h"

class TransitionNetwork : public Digraph {

    Realization X0;
    int nl;
    int M;
    int _num_paths;
    ivector _sources;
    ivector _sinks;
    std::map<int,const Transition*> transition_map;
    std::map<glp_arc*,bool> cues_map;
    std::map<glp_arc*,ivector> phi_map;

public:
    TransitionNetwork(const ChordGraph &cg, const ivector &walk, const Realization &r, const std::vector<double> &wgh, int z);
    /* constructs the transition network for walk in cg with center of gravity z and weights wgh */

    const ivector &sources() const;
    /* returns the sources of this network */

    const ivector &sinks() const;
    /* returns the sinks of this network */

    int num_levels() const;
    /* returns the number of levels in this network */

    int num_paths() const;
    /* returns the total number of paths from a source to a sink in this network */

    ivector best_path();
    /* return a cheapest path from source to sink */

    std::vector<ivector> best_paths(double &theta);
    /* returns all cheapest paths from source to sink */

    voicing realize_path(const ivector &path);
    /* returns the pitch spelling corresponding to path */

    static ivector compose(const ivector &f1, const ivector &f2);
    /* returns the composition of two permutations f1 and f2 */

    static int optimal_voicing(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh, voicing &v);
    /* finds an optimal voicing v for walk in cg and returns its gravity center on the line of fifths */

    static std::set<voicing> all_optimal_voicings(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh);
    /* returns all optimal voicings for walk in cg */

    static void arrange_voices(voicing &v);
    /* permute voices in chain so that the number of parallel fifths is minimal */

    static bool are_voicings_equivalent(const voicing &v1, const voicing &v2);
    /* returns true iff voicings v1 and v2 are equal up to a shift on the line of fifths */
};

std::ostream& operator <<(std::ostream &os, const voicing &v);
/* write voicing v (realizations are separated by newlines) to the output stream os */

#endif // TRANSITIONNETWORK_H
