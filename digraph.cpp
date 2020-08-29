/* digraph.cpp
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

#include "digraph.h"
#include <assert.h>
#include <algorithm>
#include <set>
#include <stack>
#include <queue>
#include <float.h>

#define vdata(v) ((v_data*)(v->data))
#define adata(a) ((a_data*)(a->data))
#define rdata(a) ((r_data*)(a->data))

Digraph::Digraph(bool is_weighted, bool dot_tex, bool verbose) {
    _is_weighted = is_weighted;
    _dot_tex = dot_tex;
    assert(sizeof(v_data) <= 256 && sizeof(a_data) <= 256);
    G = glp_create_graph(sizeof(v_data), sizeof(a_data));
    glp_create_v_index(G);
    P = NULL;
    _num_arcs = 0;
    _verbose = verbose;
}

Digraph::~Digraph() {
    glp_delete_graph(G);
}

bool Digraph::is_weighted() const {
    return _is_weighted;
}

int Digraph::number_of_vertices() const {
    return G->nv;
}

int Digraph::number_of_arcs() const {
    return _num_arcs;
}

int Digraph::add_vertices(int n) {
    int vi = glp_add_vertices(G, n);
    for (int i = vi; i < vi + n; ++i) {
        _vlabels[i] = std::to_string(i);
    }
    return vi;
}

void Digraph::set_vertex_name(int i, const std::string &name) {
    glp_set_vertex_name(G, i, name.c_str());
    _vlabels[i] = name;
}

int Digraph::find_vertex_by_name(const std::string &name) const {
    return glp_find_vertex(G, name.c_str());
}

glp_vertex *Digraph::vertex(int i) const {
    return G->v[i];
}

Digraph::v_data *Digraph::vertex_data(int i) const {
    assert(i > 0 || i <= G->nv);
    return vdata(G->v[i]);
}

glp_arc *Digraph::add_arc(int i, int j, double w) {
    glp_arc *a = arc(i, j);
    if (a != NULL) {
        if (_verbose)
            std::cout << " Warning: there is already an arc from vertex " << i << " to vertex " << j << std::endl;
        return a;
    }
    a = glp_add_arc(G, i, j);
    adata(a)->weight = w;
    ++_num_arcs;
    return a;
}

glp_arc *Digraph::arc(int i, int j) const {
    glp_arc *a = G->v[i]->out;
    while (a != NULL) {
        if (a->head->i == j) return a;
        a = a->t_next;
    }
    return NULL;
}

Digraph::a_data *Digraph::arc_data(int i, int j) const {
    glp_arc *a = arc(i, j);
    return arc_data(a);
}

Digraph::a_data *Digraph::arc_data(glp_arc *a) const {
    return a == NULL ? NULL : adata(a);
}

int Digraph::in_degree(int i) const {
    glp_vertex *v = G->v[i];
    glp_arc *a = v->in;
    int ret = 0;
    while (a != NULL) {
        if (adata(a)->active) ++ret;
        a = a->h_next;
    }
    return ret;
}

int Digraph::out_degree(int i) const {
    glp_vertex *v = G->v[i];
    glp_arc *a = v->out;
    int ret = 0;
    while (a != NULL) {
        if (adata(a)->active) ++ret;
        a = a->t_next;
    }
    return ret;
}

bool Digraph::bfs(int src, int dest, ivector &path) {
    assert(src > 0 && src <= G->nv && dest > 0 && dest <= G->nv &&
           vdata(G->v[src])->active && vdata(G->v[dest])->active);
    for (int i = 1; i <= G->nv; ++i) {
        vdata(G->v[i])->discovered = false;
        vdata(G->v[i])->parent = 0;
    }
    std::queue<int> Q;
    Q.push(src);
    vdata(G->v[src])->discovered = true;
    glp_vertex *v, *w;
    glp_arc *a;
    while (!Q.empty()) {
        v = G->v[Q.front()];
        Q.pop();
        if (v->i == dest) {
            path.clear();
            int i = v->i;
            while (i != 0) {
                path.push_back(i);
                i = vdata(G->v[i])->parent;
            }
            std::reverse(path.begin(), path.end());
            return true;
        }
        a = v->out;
        while (a != NULL) {
            if (adata(a)->active) {
                w = a->head;
                if (vdata(w)->active && !vdata(w)->discovered) {
                    Q.push(w->i);
                    vdata(w)->discovered = true;
                    vdata(w)->parent = v->i;
                }
            }
            a = a->t_next;
        }
    }
    return false;
}

void Digraph::yen(int src, int dest, int K, double lb, double ub, std::vector<ivector> &paths) {
    assert(lb <= ub && sizeof(r_data) <= 256);
    P = glp_create_graph(0, sizeof(r_data));
    std::set<std::pair<double, glp_vertex*> > candidates;
    std::set<std::pair<double, glp_vertex*> >::const_iterator cit;
    std::vector<glp_vertex*> final;
    std::stack<glp_arc*> inactive_arcs;
    glp_arc *a, *b;
    glp_vertex *v, *bp;
    int spur_node, i, j;
    ivector path, spur_path;
    paths.clear();
    path.reserve(G->nv);
    spur_path.reserve(G->nv);
    bool has_path;
    double pw, spw;
    if (_is_weighted) {
        path = dijkstra(src, dest);
        has_path = !path.empty();
        pw = path_weight(path);
    } else {
        has_path = bfs(src, dest, path);
        pw = path.size();
    }
    if (!has_path || (ub > 0 && pw > ub))
        return;
    glp_add_vertices(P, 1);
    bp = store_path(path, P->v[1]);
    select_path(bp);
    if (pw >= lb) final.push_back(bp);
    while (K == 0 || (int)final.size() < K) {
        restore_path(bp, src, path);
        v = P->v[1];
        for (i = 0; i + 1 < (int)path.size(); ++i) {
            spur_node = path[i];
            a = v->out;
            while (a != NULL) {
                if (rdata(a)->selected) {
                    j = rdata(a)->i;
                    b = arc(spur_node, j);
                    inactive_arcs.push(b);
                    adata(b)->active = false;
                    if (j == path[i+1])
                        v = a->head;
                }
                a = a->t_next;
            }
            if (_is_weighted) {
                pw = path_weight(ivector(path.begin(), path.begin() + i + 1));
                spur_path = dijkstra(spur_node, dest);
                has_path = !spur_path.empty();
                if (has_path)
                    spw = path_weight(spur_path);
            } else {
                pw = i;
                has_path = bfs(spur_node, dest, spur_path);
                if (has_path)
                    spw = spur_path.size();
            }
            if (has_path)
                candidates.insert(std::make_pair(pw + spw, store_path(spur_path, v->in->tail)));
            vdata(G->v[spur_node])->active = false;
        }
        for (ivector::const_iterator it = path.begin(); it + 1 != path.end(); ++it) {
            vdata(G->v[*it])->active = true;
        }
        while (!inactive_arcs.empty()) {
            adata(inactive_arcs.top())->active = true;
            inactive_arcs.pop();
        }
        if (candidates.empty()) break;
        cit = candidates.begin();
        if (ub > 0 && cit->first > ub) break;
        bp = cit->second;
        select_path(bp);
        if (cit->first >= lb) final.push_back(bp);
        candidates.erase(cit);
    }
    for (std::vector<glp_vertex*>::const_iterator it = final.begin(); it != final.end(); ++it) {
        restore_path(*it, src, path);
        paths.push_back(path);
    }
    glp_delete_graph(P);
}

ivector Digraph::dijkstra(int src, int dest) {
    assert(src > 0 && src <= G->nv && (dest == 0 || (dest > 0 && dest <= G->nv)));
    assert(vdata(G->v[src])->active && (dest == 0 || vdata(G->v[dest])->active));
    int i, k;
    ivector Q;
    Q.reserve(G->nv);
    std::vector<bool> popped(G->nv+1, false);
    glp_vertex *u, *v;
    glp_arc *a;
    double mindist, alt;
    for (i = 1; i <= G->nv; ++i) {
        v = G->v[i];
        if (!vdata(v)->active)
            continue;
        vdata(v)->dist = i == src ? 0 : DBL_MAX;
        vdata(v)->parent = 0;
        Q.push_back(i);
    }
    while (!Q.empty()) {
        u = NULL;
        for (ivector::const_iterator it = Q.begin(); it!=Q.end(); ++it) {
            if (u == NULL || vdata(G->v[*it])->dist < mindist) {
                u = G->v[*it];
                mindist = vdata(u)->dist;
                k = (int)(it - Q.begin());
            }
        }
        Q.erase(Q.begin() + k);
        if (u->i == dest)
            return get_path(dest);
        popped[u->i] = true;
        a = u->out;
        while (a != NULL) {
            if (adata(a)->active) {
                v = a->head;
                if (vdata(v)->active && !popped[v->i]) {
                    alt = vdata(u)->dist + adata(a)->weight;
                    if (alt < vdata(v)->dist) {
                        vdata(v)->dist = alt;
                        vdata(v)->parent = u->i;
                    }
                }
            }
            a = a->t_next;
        }
    }
    return ivector(0);
}

ivector Digraph::get_path(int dest) const {
    ivector path;
    int i = dest;
    while(i > 0) {
        path.push_back(i);
        i = vdata(G->v[i])->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

double Digraph::path_weight(const ivector &path) const {
    double ret = 0;
    for (ivector::const_iterator it = path.begin() + 1; it != path.end(); ++it) {
        ret += arc_data(*(it - 1), *it)->weight;
    }
    return ret;
}

void Digraph::enable_all_vertices(bool yes) {
    for (int i = 1; i <= G->nv; ++i) {
        vdata(G->v[i])->active = yes;
    }
}

void Digraph::enable_all_arcs(bool yes) {
    glp_vertex *v;
    glp_arc *a;
    for (int i = 1; i <= G->nv; ++i) {
        v = G->v[i];
        a = v->out;
        while (a != NULL) {
            adata(a)->active = yes;
            a = a->t_next;
        }
    }
}

glp_vertex *Digraph::store_path(const ivector &path, glp_vertex *root) {
    glp_vertex *v = root;
    glp_arc *a;
    int i, j, n = path.size();
    for (i = 1; i < n; ++i) {
        a = v->out;
        while (a != NULL) {
            if (rdata(a)->i == path[i]) break;
            a = a->t_next;
        }
        if (a != NULL) {
            v = a->head;
            continue;
        }
        j = glp_add_vertices(P, 1);
        a = glp_add_arc(P, v->i, j);
        rdata(a)->i = path[i];
        rdata(a)->selected = false;
        v = P->v[j];
    }
    return v;
}

void Digraph::select_path(glp_vertex *top) {
    glp_arc *a;
    glp_vertex *v = top;
    while ((a = v->in) != NULL) {
        if (rdata(a)->selected) break;
        rdata(a)->selected = true;
        v = a->tail;
    }
}

void Digraph::restore_path(glp_vertex *top, int src, ivector &path) {
    glp_arc *a;
    glp_vertex *v = top;
    path.clear();
    while ((a = v->in) != NULL) {
        path.push_back(rdata(a)->i);
        v = a->tail;
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());
}

Matrix Digraph::adjacency_matrix() const {
    Matrix ret(G->nv);
    glp_vertex *v;
    glp_arc *a;
    for (int i = 1; i <= G->nv; ++i) {
        v = G->v[i];
        a = v->out;
        while (a != NULL) {
            if (adata(a)->active)
                ret.set_element(i, a->head->i, 1.0);
            a = a->t_next;
        }
    }
    return ret;
}

bool Digraph::export_dot(const char *filename) const {
    std::ofstream dot;
    dot.open(filename);
    if (!dot.is_open())
        return false;
    dot << "digraph {\n";
    /* output vertices */
    for (int i = 1; i <= G->nv; ++i) {
        dot << "  v" << i;
        if (_dot_tex)
            dot << " [texlbl=\"$" << _vlabels.at(i) << "$\"];\n";
        else
            dot << " [label=\"" << _vlabels.at(i) << "\"];\n";
    }
    /* output arcs */
    for (int i = 1; i <= G->nv; ++i) {
        for (int j = 1; j <= G->nv; ++j) {
            if (i == j)
                continue;
            glp_arc *a = arc(i, j);
            if (a != NULL) {
                dot << "  v" << i << " -> v" << j;
                if (_is_weighted)
                    dot << " [weight=" << adata(a)->weight << "]";
                dot << ";\n";
            }
        }
    }
    dot << "}\n";
    dot.close();
    return true;
}

std::ostream& operator <<(std::ostream &os, const ivector &v) {
    int n = v.size(), i = 0;
    for (ivector::const_iterator it = v.begin(); it != v.end(); ++it) {
        ++i;
        os << *it;
        if (i != n)
            os << ",";
    }
    return os;
}
