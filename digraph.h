/* digraph.h
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

#ifndef DIGRAPH_H
#define DIGRAPH_H

#include "matrix.h"
#include <glpk.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>

typedef std::vector<int> ivector;

class Digraph {

private:
    typedef struct {
        bool active;
        int parent;
        bool discovered;
        double dist;
    } v_data;

    typedef struct {
        bool active;
        double weight;
    } a_data;

    typedef struct {
        int i;
        bool selected;
    } r_data;

    glp_graph *G;
    glp_graph *P;
    int _num_arcs;
    bool _verbose;
    bool _dot_tex;
    bool _is_weighted;
    std::map<int,std::string> _vlabels;
    glp_vertex *store_path(const ivector &path, glp_vertex *root);
    static void select_path(glp_vertex *top);
    static void restore_path(glp_vertex *top, int src, ivector &path);

public:
    Digraph(bool is_weighted, bool dot_tex, bool verbose);
    /* constructor
     *  - is_weighted should be set to true to use arc weights
     *  - if use_vertex_labels = true, then vertices are searchable by string labels
     *  - if dot_tex = true, then dot exports vertex labels as texlbl for processing with dot2tex
     *  - if verbose = true, then messages are printed to standard output
     */

    ~Digraph();
    /* destructor */

    bool is_weighted() const;
    /* returns true iff this graph is weighted */

    int number_of_vertices() const;
    /* returns the number of vertices in this graph */

    int number_of_arcs() const;
    /* returns the number of arcs in this graph */

    int add_vertices(int n);
    /* adds n vertices and returns the index of the first vertex (indices are 1-based) */

    void set_vertex_name(int i, const std::string &name);
    /* assigns label to name of the i-th vertex */

    int find_vertex_by_name(const std::string &name) const;
    /* returns the index of the vertex with the given name */

    glp_vertex *vertex(int i) const;
    /* returns the i-th vertex */

    v_data *vertex_data(int i) const;
    /* returns the data for vertex with index i */

    glp_arc *add_arc(int i, int j, double w = 1.0);
    /* adds an arc from i-th to j-th vertex with weight w */

    glp_arc *arc(int i, int j) const;
    /* returns the arc from i-th to j-th vertex */

    a_data *arc_data(int i, int j) const;
    /* returns the data for arc (vi,vj) */

    a_data *arc_data(glp_arc *a) const;
    /* returns the data for arc a */

    int in_degree(int i) const;
    /* returns the in-degree for the i-th vertex */

    int out_degree(int i) const;
    /* returns the out-degree for the i-th vertex */

    bool bfs(int src, int dest, ivector &path);
    /* finds a path from src to dest found by breadth-first search, return true iff one exists */

    void yen(int src, int dest, int K, double lb, double ub, std::vector<ivector> &paths);
    /* An implementation of Yen's algorithm for K shortest paths from src to dest vertex.
     *  - the parameters lb and ub are the lower and the upper bound for path length/weight
     *  - if K = 0, there is no limit; the algorithm finds all paths in this case
     */

    ivector dijkstra(int src, int dest = 0);
    /* returns a shortest path from src to dest using Dijkstra's algorithm
     *  - if dest = 0, then all shortest paths from src are computed and can
     *    be retrieved by get_path routine (the return value is irrelevant in this case) */

    ivector get_path(int dest) const;
    /* returns the shortest path from src to dest as computed by dijkstra(src, 0) */

    double path_weight(const ivector &path) const;
    /* returns the weight of path in this graph */

    void enable_all_vertices(bool yes = true);
    /* makes all vertices active (searchable) */

    void enable_all_arcs(bool yes = true);
    /* makes all arcs active (traversable) */

    Matrix adjacency_matrix() const;
    /* returns the adjacency matrix (of this network) */

    bool export_dot(const char* filename) const;
    /* outputs the chord graph in dot format to file 'filename' */
};

std::ostream& operator <<(std::ostream &os, const ivector &v);

#endif // DIGRAPH_H
