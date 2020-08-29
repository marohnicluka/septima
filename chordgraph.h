/* chordgraph.h
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

#ifndef CHORDGRAPH_H
#define CHORDGRAPH_H

#include "transition.h"
#include "domain.h"
#include "digraph.h"

typedef std::map<std::pair<int,int>,std::vector<ivector> > pathmap;
typedef std::vector<std::pair<Realization, bool> > pitchSpelling;

class ChordGraph : public Digraph {

    int M; // class index
    Domain _support;
    bool _allows_aug;
    std::map<int,Chord> chord_map;
    std::map<glp_arc*,std::set<Transition> > transition_map;

    struct path_comp {
        bool operator ()(const ivector &p, const ivector &q) const {
            if (p.size() == q.size())
                return p < q;
            return p.size() < q.size();
        }
    };

    static int rand_int(int n);

    static ivector rand_perm(int n);

    void make_acyclic(const ivector &perm);
    /* makes this network acyclic by making some arcs inactive */

public:
    ChordGraph(const std::vector<Chord> &chords, int k, const Domain &sup, PreparationScheme p, bool aug, bool use_labels,
               bool is_weighted = false, bool dot_tex = false, bool verbose = false);
    /* constructs the chord graph using chords as vertices
     *  - if a chord does not have realizations in sup, then it is not added to the graph
     *  - k is the class index
     *  - there is an arc from c to d iff there exists an elementary transition from c to d of class k
     *  - if use_labels = true, then vertices are labeled by chord names, else by 1, 2, ...
     *  - p is preparation scheme (see transition.h)
     *  - if aug = true, augmented sixths are allowed
     */

    int class_index() const;
    /* returns the class index M */

    const Domain &support() const;
    /* returns the support domain */

    bool allows_augmented_sixths() const;
    /* returns true iff augmented-sixth realizations are allowed in this graph */

    const std::set<Transition> &transitions(glp_arc *a) const;
    /* returns the list transitions corresponding to a */

    int find_vertex_by_chord(const Chord &c) const;
    /* returns the index of the vertex corresponding to c, or 0 if no such vertex exists in this graph */

    const Chord &vertex2chord(int i) const;
    /* returns the reference to chord represented by the i-th vertex */

    bool pitch_spelling(const char *prog[], int len, int &z0, double spread_weight, double vl_weight, double aug_weight, pitchSpelling &ps) const;
    /* finds the pitch spelling for progression prog of seventh chords, given as a list of names
     *  - returns true iff prog is a walk in this graph
     *  - z0 is the gravity center on the line of fifths
     *  - spread_weight, vl_weight and aug_weight are weight parameters for xi, eta and zeta, respectively
     */

    bool best_pitch_spellings(const char *prog[], int len, double spread_weight, double vl_weight, double aug_weight, std::set<pitchSpelling> &ps) const;
    /* finds all optimal pitch spellings for prog with respect to the given weight parameters
     *  - returns true iff prog is a walk in this graph
     */

    void shortest_paths(int src, int dest, std::vector<ivector> &paths);
    /* finds all shortest s-t paths */

    void all_shortest_paths(pathmap &path_map);
    /* finds all shortest paths and map them with pairs of endpoint vertex indices as keys */

    double closeness_centrality(int i);
    /* computes the closeness centrality for the i-th vertex */

    double betweenness_centrality(int i, const pathmap &path_map) const;
    /* computes the betweenness centrality for the i-th vertex */

    double communicability_betweenness_centrality(int k) const;
    /* computes the communicability betweenness centrality for the k-th vertex */

    double katz_centrality(int k, bool rev = false) const;
    /* computes the Katz centrality for the k-th vertex */

    void find_fixed_length_paths(int src, int dest, int len, int limit, std::vector<ivector> &paths);
    /* finds at most k=limit paths of length len from src to dest */
};

#endif // CHORDGRAPH_H
