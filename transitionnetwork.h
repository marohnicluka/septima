/* transitionnetwork.h
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

    static double cog_offset_factor(int z);
    /* returns 1+z^2/50 which is the path weight factor depending on the center of gravity z */

public:
    TransitionNetwork(const ChordGraph &cg, const ivector &walk, const Realization &r, const std::vector<double> &wgh, int z,
                      bool verbose = false);
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

    pitchSpelling realize_path(const ivector &path);
    /* returns the pitch spelling corresponding to path */

    static ivector compose(const ivector &f1, const ivector &f2);
    /* returns the composition of two permutations f1 and f2 */

    static int optimal_pitch_spelling(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh, pitchSpelling &ps);
    /* finds an optimal pitch spelling ps for walk in cg and returns its gravity center on the line of fifths */

    static std::set<pitchSpelling> all_optimal_pitch_spellings(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh);
    /* returns all optimal pitch spellings for walk in cg */

    static void arrange_voices(pitchSpelling &ps);
    /* permute voices in chain so that the number of parallel fifths is minimal */

    static bool are_pitch_spellings_equivalent(const pitchSpelling &ps1, const pitchSpelling &ps2);
    /* returns true iff ps1 and ps2 are equal up to a shift on the line of fifths */
};

std::ostream& operator <<(std::ostream &os, const pitchSpelling &ps);
/* feed pitch spelling ps to the output stream os */

#endif // TRANSITIONNETWORK_H
