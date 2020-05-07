/* chordgraph.h
 *
 * Copyright (c) 2020 Luka Marohnić
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

#ifndef CHORDGRAPH_H
#define CHORDGRAPH_H

#include "transition.h"
#include "matrix.h"
#include <glpk.h>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

typedef std::vector<int> ivector;
typedef std::map<std::pair<int,int>,std::vector<ivector> > pathmap;

class Network {

    typedef struct {
        Chord chord;
        bool active;
        int parent;
        bool discovered;
        double dist;
    } v_data;
    typedef struct {
        double weight;
        bool active;
        std::vector<Transition> transitions;
    } a_data;
    typedef struct {
        int i;
        bool selected;
    } r_data;
    typedef struct {
        const Transition *transition;
        int parent;
        int level;
        double dist;
    } tv_data;
    typedef struct {
        std::vector<int> vl;
        int mandatory_cues;
        double weight;
    } ta_data;
    struct path_comp {
        bool operator ()(const ivector &p, const ivector &q) const {
            if (p.size() == q.size())
                return p < q;
            return p.size() < q.size();
        }
    };
    glp_graph *G;
    glp_graph *P;
    std::stringstream dot;
    int narcs;
    static int rand_int(int n);
    static ivector rand_perm(int n);
    double path_weight(const ivector &path) const;
    glp_vertex *store_path(const ivector &path, glp_vertex *root);
    void select_path(glp_vertex *top);
    void restore_path(glp_vertex *top, int src, ivector &path);

public:
    Network(const std::vector<Chord> &chords); // construct the chord graph from chords
    ~Network();

    bool to_dot(const char* filename) const;
    /* output network to dot file */

    static void print_list(const ivector &lst);
    /* print list of integers to stdout */

    int vertex_index(const char *name) const;
    /* Return the vertex index corresponding to the given name (indices are 1-based).
     * The name must be entered in form "<pitch>:<chordtype>", where <chordtype> is
     * d7 (dominant seventh), hdim7 (half-diminished seventh), dim7 (diminished seventh),
     * m7 (minor seventh), and maj7 (major seventh)
     */

    glp_vertex *vertex(const Chord &c) const;
    /* return the vertex corresponding to the chord c */

    v_data *vertex_data(int i) const;
    /* return the data for vertex with index i */

    glp_arc *arc(int i, int j) const;
    /* return the arc from i-th to j-th vertex */

    a_data *arc_data(int i, int j) const;
    /* return the data for arc (vi,vj) */

    int in_degree(int i) const;
    /* return the in-degree for the i-th vertex */

    int out_degree(int i) const;
    /* return the out-degree for the i-th vertex */

    bool bfs(int src, int dest, ivector &path);
    /* find a path from src to dest found by breadth-first search, return true iff one exists */

    void yen(int src, int dest, int K, int lb, int ub, std::vector<ivector> &paths);
    /* An implementation of Yen's algorithm for K shortest paths from src to dest vertex.
     * The parameters lb and ub are the lower and the upper bound for path length.
     * If K is zero, there is no limit; the algorithm finds all paths in this case.
     */

    void shortest_paths(int src, int dest, std::vector<ivector> &paths);
    /* find all shortest s-t paths */

    void all_shortest_paths(pathmap &path_map);
    /* find all shortest paths and map them with pairs of endpoint vertex indices as keys */


    double closeness_centrality(int i);
    /* compute the closeness centrality for the i-th vertex */

    double betweenness_centrality(int i, const pathmap &path_map);
    /* compute the betweenness centrality for the i-th vertex */

    void enable_all_vertices(bool yes = true);
    /* make all vertices active (searchable) */

    void enable_all_arcs(bool yes = true);
    /* make all arcs active (traversable) */

    Matrix adjacency_matrix() const;
    static Matrix adjacency_matrix(glp_graph *g);
    /* return the adjacency matrix (of this network) */

    void make_acyclic(const ivector &perm);
    /* make this network acyclic by making some arcs inactive */

    void find_fixed_length_paths(int src, int dest, int len, int limit, std::vector<ivector> &paths);
    /* find at most k=limit paths of length len from src to dest vertex */

    ivector static compose(const ivector &f1, const ivector &f2);
    /* return the composition of permutations f1 and f2 in S4 */

    glp_graph *transition_network(const ivector &trail, const Realization &R0, const std::vector<double> &wgh, int key, int &num_paths) const;
    /* Return the transition network for the given trail of chords.
     * R0 is the initial realization (must correspond to the first chord).
     * wgh is a vector of doubles with 4 entries w1, w2, w3, w4 (weights of penalty functions z1, z2, z3, z4.
     * key is an integer between -7 and 7. It indicates the number of sharps (key>0) or flats (key<0) in the signature.
     * Total number of possible paths from source to sink is written in num_paths.
     */

    /* transition network utilities: */
    static ivector dijkstra(glp_graph *tn, int src, int dest = 0);
    static void tn_print_path(glp_graph *tn, const Realization &R0, const ivector &path);
    static double tn_path_size(glp_graph *tn, const ivector &path);
    static void tn_get_sources_and_sinks(glp_graph *tn, ivector &sources, ivector &sinks);
    static void tn_all_path_weights(glp_graph *tn, std::ofstream &file, glp_vertex *from = NULL, double weight = 0);    
};

#endif // CHORDGRAPH_H
