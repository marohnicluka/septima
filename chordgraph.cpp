/* chordgraph.cpp
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

#include "chordgraph.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <stack>
#include <queue>
#include <map>
#include <float.h>
#include <math.h>

#define vdata(v) ((v_data*)(v->data))
#define adata(a) ((a_data*)(a->data))
#define rdata(a) ((r_data*)(a->data))
#define t_vdata(v) ((tv_data*)(v->data))
#define t_adata(a) ((ta_data*)(a->data))

Network::Network(const std::vector<Chord> &chords) {
    assert(sizeof(v_data) <= 256 && sizeof(a_data) <= 256);
    G = glp_create_graph(sizeof(v_data), sizeof(a_data));
    int i, j, n = chords.size();
    std::cout << "Generating chord graph on " << n << " vertices..." << std::endl;
    glp_add_vertices(G, n);
    glp_create_v_index(G);
    dot << "digraph {\n";
    for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
        i = 1 + int(it - chords.begin());
        glp_set_vertex_name(G, i, it->to_string().c_str());
        vertex_data(i)->chord = *it;
        dot << "  v" << i << " [label=\"" << it->to_string() << "\"];\n";
    }
    narcs = 0;
    int ntrans = 0, up_trans = 0, down_trans = 0, aug = 0, vld, wgh, vls;
    bool is_aug, slide;
    std::vector<Transition> bt;
    std::map<int,int> vl_type, vls_map;
    std::set<int> aug_vertices;
    ivector numpt;
    std::map<std::pair<int,int>,ivector> cty;
    for (i = 1; i <= n; ++i) {
        for (j = 1; j <= n; ++j) {
            if (i == j) continue;
            bt = Transition::generate_parsimonious(vertex_data(i)->chord, vertex_data(j)->chord);
            if (bt.empty()) continue;
            numpt.push_back(bt.size());
            cty[std::make_pair(vertex_data(i)->chord.type(),vertex_data(j)->chord.type())].push_back(bt.size());
            wgh=RAND_MAX;
            is_aug=true;
            slide=true;
            for (std::vector<Transition>::const_iterator it = bt.begin(); it != bt.end(); ++it) {
                if (is_aug && !it->first().is_augmented() && !it->second().is_augmented())
                    is_aug=false;
                vld = it->vld();
                vls_map[it->vls()]++;
                if (slide && it->vl_type() !=1)
                    slide=false;
                ++vl_type[it->vl_type()];
                //std::cout << it->vls(true) << ",";
                if (vld > 0)
                    ++up_trans;
                else if (vld < 0)
                    ++down_trans;
                vls = it->vls();
                if (wgh > vls)
                    wgh = vls;
            }
            assert(wgh < RAND_MAX);
            glp_arc *a = glp_add_arc(G, i, j);
            ++narcs;
            adata(a)->transitions = bt;
            ntrans += adata(a)->transitions.size();
            adata(a)->weight = wgh;
            dot << "  v" << i << " -> v" << j << " [weight=" << wgh << "];\n";
        }
    }
    dot << "}\n";
    std::cout << "Number of arcs: " << narcs << std::endl;
    std::cout << "Number of transitions: " << ntrans << std::endl;
}

Network::~Network() {
    glp_delete_graph(G);
}

bool Network::to_dot(const char *filename) const {
    std::ofstream dot_file;
    dot_file.open(filename);
    if (!dot_file.is_open())
        return false;
    dot_file << dot.str();
    dot_file.close();
    return true;
}

glp_vertex *Network::vertex(const Chord &c) const {
    int i = glp_find_vertex(G, c.to_string().c_str());
    if (i == 0) return NULL;
    assert(vertex_data(i)->chord == c);
    return G->v[i];
}

int Network::vertex_index(const char *name) const {
    return glp_find_vertex(G, name);
}

Network::v_data *Network::vertex_data(int i) const {
    assert(i > 0 || i <= G->nv);
    return vdata(G->v[i]);
}

glp_arc *Network::arc(int i, int j) const {
    glp_arc *a = G->v[i]->out;
    while (a != NULL) {
        if (a->head->i == j) return a;
        a = a->t_next;
    }
    return NULL;
}

Network::a_data *Network::arc_data(int i, int j) const {
    glp_arc *a = arc(i, j);
    return a == NULL ? NULL : adata(a);
}

int Network::in_degree(int i) const {
    glp_vertex *v = G->v[i];
    glp_arc *a = v->in;
    int ret = 0;
    while (a != NULL) {
        if (adata(a)->active) ++ret;
        a = a->h_next;
    }
    return ret;
}

int Network::out_degree(int i) const {
    glp_vertex *v = G->v[i];
    glp_arc *a = v->out;
    int ret = 0;
    while (a != NULL) {
        if (adata(a)->active) ++ret;
        a = a->t_next;
    }
    return ret;
}

bool Network::bfs(int src, int dest, ivector &path) {
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

double Network::path_weight(const ivector &path) const {
    double ret = 0;
    for (ivector::const_iterator it = path.begin() + 1; it != path.end(); ++it) {
        ret += arc_data(*(it - 1), *it)->weight;
    }
    return ret;
}

void Network::enable_all_vertices(bool yes) {
    for (int i = 1; i <= G->nv; ++i) {
        vdata(G->v[i])->active = yes;
    }
}

void Network::enable_all_arcs(bool yes) {
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

glp_vertex *Network::store_path(const ivector &path, glp_vertex *root) {
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

void Network::select_path(glp_vertex *top) {
    glp_arc *a;
    glp_vertex *v = top;
    while ((a = v->in) != NULL) {
        if (rdata(a)->selected) break;
        rdata(a)->selected = true;
        v = a->tail;
    }
}

void Network::restore_path(glp_vertex *top, int src, ivector &path) {
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

void Network::yen(int src, int dest, int K, int lb, int ub, std::vector<ivector> &paths) {
    assert(lb <= ub && sizeof(r_data) <= 256);
    P = glp_create_graph(0, sizeof(r_data));
    std::set<std::pair<int, glp_vertex*> > candidates;
    std::set<std::pair<int, glp_vertex*> >::const_iterator cit;
    std::vector<glp_vertex*> final;
    std::stack<glp_arc*> inactive_arcs;
    glp_arc *a, *b;
    glp_vertex *v, *bp;
    int spur_node, i, j;
    ivector path, spur_path;
    paths.clear();
    path.reserve(G->nv);
    spur_path.reserve(G->nv);
    if (!bfs(src, dest, path) || (ub > 0 && (int)path.size() > ub))
        return;
    glp_add_vertices(P, 1);
    bp = store_path(path, P->v[1]);
    select_path(bp);
    if ((int)path.size() >= lb) final.push_back(bp);
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
                    if (j == path[i + 1])
                        v = a->head;
                }
                a = a->t_next;
            }
            if (bfs(spur_node, dest, spur_path))
                candidates.insert(std::make_pair(i + spur_path.size(), store_path(spur_path, v->in->tail)));
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

void Network::shortest_paths(int src, int dest, std::vector<ivector> &paths) {
    if (src == dest)
        return;
    int k = 0;
    while (paths.empty()) {
        ++k;
        enable_all_arcs();
        enable_all_vertices();
        yen(src, dest, 0, k, k, paths);
    }
}

void Network::all_shortest_paths(pathmap &path_map) {
    int n = G->nv;
    path_map.clear();
    int total_paths = 0, total_length = 0, len1_count = 0, len2_count = 0, len;
    for (int j = 1; j <= n; ++j) {
        for (int k = 1; k <= n; ++k) {
            if (j == k)
                continue;
            std::vector<ivector> &paths = path_map[std::make_pair(j, k)];
            shortest_paths(j, k, paths);
            total_paths += paths.size();
            for (std::vector<ivector>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
                len = it->size() - 1;
                total_length += len;
                if (len == 1)
                    len1_count++;
                else len2_count++;
            }
        }
    }
    std::cout << "TOTAL SHORTEST PATHS: " << total_paths << std::endl;
    std::cout << "LEN1: " << len1_count << ", LEN2: " << len2_count
              << ", AVERAGE LEN: " << ((double)total_length)/total_paths << std::endl;
}

double Network::betweenness_centrality(int i, const pathmap &path_map) {
    int n = G->nv, m, p, N = 0;
    double ret = 0;
    for (int j = 1; j <= n; ++j) {
        if (i == j)
            continue;
        for (int k = 1; k <= n; ++k){
            if (j == k || k == i)
                continue;
            const std::vector<ivector> &paths = path_map.at(std::make_pair(j, k));
            m = paths.size();
            p = 0;
            for (std::vector<ivector>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
                if (std::find(it->begin(), it->end(), i) != it->end())
                    ++p;
            }
            ret += (double)p/(double)m;
            ++N;
        }
    }
    return ret/N;
}

double Network::closeness_centrality(int i) {
    int n=G->nv, d = 0;
    ivector path;
    for (int j = 1; j <= n; ++j) {
        if (i == j)
            continue;
        bfs(i, j, path);
        d += path.size() - 1;
    }
    return (double)(n - 1)/(double)d;
}

Matrix Network::adjacency_matrix() const {
    Matrix ret(G->nv);
    glp_vertex *v;
    glp_arc *a;
    for (int i = 1; i <= G->nv; ++i) {
        v = G->v[i];
        a = v->out;
        while (a != NULL) {
            if (adata(a)->active)
                ret.set_element(i, a->head->i, 1);
            a = a->t_next;
        }
    }
    return ret;
}

Matrix Network::adjacency_matrix(glp_graph *g) {
    Matrix ret(g->nv);
    glp_vertex *v;
    glp_arc *a;
    for (int i = 1; i <= g->nv; ++i) {
        v = g->v[i];
        a = v->out;
        while (a != NULL) {
            ret.set_element(i, a->head->i, 1);
            a = a->t_next;
        }
    }
    return ret;
}

int Network::rand_int(int n) {
    return rand() % n;
}

ivector Network::rand_perm(int n) {
    ivector p(n + 1, 0);
    int i, j, tmp;
    for (i = 1; i <= n; ++i) {
        p[i] = i;
    }
    for (i = 1; i < n; ++i) {
        j = i + rand_int(n - i + 1);
        if (i != j) {
            tmp = p[i];
            p[i] = p[j];
            p[j] = tmp;
        }
    }
    return p;
}

void Network::print_list(const ivector &lst) {
    for (ivector::const_iterator it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

void Network::make_acyclic(const ivector &perm) {
    glp_vertex *v;
    glp_arc *a;
    int i, j;
    for (i = 1; i <= G->nv; ++i) {
        v = G->v[i];
        a = v->out;
        while (a != NULL) {
            j = a->head->i;
            if (perm[i] > perm[j])
                adata(a)->active = false;
            a = a->t_next;
        }
    }
}

void Network::find_fixed_length_paths(int src, int dest, int len, int limit, std::vector<ivector> &paths) {
    assert(len > 1 && src > 0 && src <= G->nv && dest > 0 && dest <= G->nv);
    enable_all_vertices();
    enable_all_arcs();
    paths.clear();
    ivector p;
    int i, j, k, pc, s;
    std::set<ivector, path_comp> path_set;
    std::set<ivector> used_perm;
    std::set<std::pair<double, ivector> > perm_set;
    s = 0;
    while ((pc = path_set.size()) < limit) {
        while (1) {
            while (1) {
                p = rand_perm(G->nv);
                if (used_perm.find(p) == used_perm.end())
                    break;
            }
            used_perm.insert(p);
            make_acyclic(p);
            Matrix A = adjacency_matrix(), B = A;
            j = A.element(src, dest);
            for (i = 2; i <= len; ++i) {
                B.mul(A, src, i == len ? dest : 0);
                j += B.element(src, dest);
            }
            enable_all_arcs();
            if ((k = B.element(src, dest)) > 0 && j <= 2.5e5) {
                p.push_back(k);
                perm_set.insert(std::make_pair(j / (double)k, p));
                s += k;
                if (s > limit) break;
            }
            std::cout << "\r" << (s * 100) / limit << " %";
        }
        if (perm_set.empty()) continue;
        const std::pair<double, ivector> &bp = *perm_set.begin();
        s -= bp.second.back();
        std::cout << "\r[" << pc << ", " << perm_set.size() << "] Run Yen for "
                  << bp.first * bp.second.back() << " paths...  ";
        make_acyclic(bp.second);
        fflush(stdout);
        std::vector<ivector> yp;
        yen(src, dest, 0, len, len, yp);
        for (std::vector<ivector>::const_iterator it = yp.begin(); it != yp.end(); ++it) {
            path_set.insert(*it);
            if ((int)path_set.size() == limit) break;
        }
        perm_set.erase(perm_set.begin());
        enable_all_arcs();
    }
    std::cout << "\rFound " << limit << " paths                                           " << std::endl;
    for (std::set<ivector, path_comp>::const_iterator it = path_set.begin(); it != path_set.end(); ++it) {
        paths.push_back(*it);
        if ((int)paths.size() == limit) break;
    }
}

glp_graph *Network::transition_network(const ivector &trail, const Realization &R0, const std::vector<double> &wgh, int key, int &num_paths) const {
    assert(sizeof(tv_data) <= 256 && sizeof(ta_data) <= 256);
    glp_graph *tn = glp_create_graph(sizeof(tv_data), sizeof(ta_data));
    glp_vertex *v, *w;
    glp_arc *a;
    int k, p, vi, n = trail.size() - 1;
    std::vector<int> f;
    num_paths = 1;
    std::map<int,std::vector<glp_vertex*> > levels;
    for (p = n; p-->0;) {
        a = arc(trail[p], trail[p + 1]);
        if (a == NULL)
            return NULL;
        std::vector<Transition> &ta = adata(a)->transitions;
        vi = glp_add_vertices(tn, ta.size());
        num_paths *= ta.size();
        for (std::vector<Transition>::const_iterator it = ta.begin(); it != ta.end(); ++it) {
            v = tn->v[vi + int(it - ta.begin())];
            t_vdata(v)->transition = &(*it);
            t_vdata(v)->level = p + 1;
            levels[p+1].push_back(v);
        }
    }
    for (p = 1; p < n; ++p) {
        std::vector<glp_vertex*> &lev1 = levels[p], &lev2 = levels[p+1];
        for (std::vector<glp_vertex*>::const_iterator it = lev1.begin(); it != lev1.end(); ++it) {
            v = *it;
            const Transition &t1 = *(t_vdata(v)->transition);
            for (std::vector<glp_vertex*>::const_iterator jt = lev2.begin(); jt != lev2.end(); ++jt) {
                w = *jt;
                const Transition &t2 = *(t_vdata(w)->transition);
                assert(t2.is_link(t1.second(), k, f));
                a = glp_add_arc(tn, v->i, w->i);
                t_adata(a)->vl = f;
                t_adata(a)->mandatory_cues = k;
                t_adata(a)->weight = wgh[0] * k / (4.0*n);
                if (p == 1) {
                    assert(t1.is_link(R0, k, f));
                    t_adata(a)->mandatory_cues += k;
                    t_adata(a)->weight += wgh[0] * k / (4.0*n);
                    t_adata(a)->weight += wgh[1] * (t1.second().acc_weight(key) +
                                                    t2.second().acc_weight(key)) / (4.0*(n+1));
                    t_adata(a)->weight += wgh[2] * (t1.vls() + t2.vls()) / (7.0*n);
                    t_adata(a)->weight += wgh[3] * (int(t1.second().is_augmented()) +
                                                    int(t2.second().is_augmented())) / (n+1.0);
                } else {
                    t_adata(a)->weight += wgh[1] * t2.second().acc_weight(key) / (4.0*(n+1));
                    t_adata(a)->weight += wgh[2] * t2.vls() / (7.0*n);
                    t_adata(a)->weight += wgh[3] * int(t2.second().is_augmented()) / (n+1.0);
                }
            }
        }
    }
    return tn;
}

ivector Network::compose(const ivector &f1, const ivector &f2) {
    assert(f1.size() == 4 && f2.size() == 4);
    ivector tmp(4);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = f2[f1[i]];
    }
    return tmp;
}

void Network::tn_get_sources_and_sinks(glp_graph *tn, ivector &sources, ivector &sinks) {
    for (int i = 1; i <= tn->nv; ++i) {
        if (tn->v[i]->in == NULL)
            sources.push_back(i);
        else if (tn->v[i]->out == NULL)
            sinks.push_back(i);
    }
}

void Network::tn_all_path_weights(glp_graph *tn, std::ofstream &file, glp_vertex *v, double weight) {
    glp_arc *a;
    if (v == NULL) {
        ivector sources, sinks;
        tn_get_sources_and_sinks(tn, sources, sinks);
        for (ivector::const_iterator it = sources.begin(); it != sources.end(); ++it) {
            v = tn->v[*it];
            a = v->out;
            while (a != NULL) {
                tn_all_path_weights(tn, file, a->head, t_adata(a)->weight);
                a = a->t_next;
            }
        }
    } else {
        a = v->out;
        if (a == NULL)
            file << weight << " ";
        else while (a != NULL) {
            tn_all_path_weights(tn, file, a->head, weight + t_adata(a)->weight);
            a = a->t_next;
        }
    }
}

void Network::tn_print_path(glp_graph *tn, const Realization &R0, const ivector &path) {
    glp_vertex *v, *w;
    glp_arc *a;
    int n = path.size();
    ivector f(4),f0;
    for (int i = 0; i < 4; ++i) f[i] = i;
    for (int i = 0; i < n; ++i) {
        v = tn->v[path[i]];
        const Transition &t = *(t_vdata(v)->transition);
        Realization r1 = t.first(), r2 = t.second();
        r1.arrange(f);
        r2.arrange(f);
        if (i == 0) {
            std::cout << R0.to_string() << std::endl << r2.to_string();
        } else {
            std::cout << r2.to_string();
        }
        if (i != n - 1) {
            w = tn->v[path[i+1]];
            a = v->out;
            while (a != NULL) {
                if (a->head == w) break;
                a = a->t_next;
            }
            assert(a != NULL);
            f = compose(f, t_adata(a)->vl);
            if (t_adata(a)->mandatory_cues > 0) {
                const Transition &u = *(t_vdata(w)->transition);
                Realization r = u.first();
                r.arrange(f);
                std::cout << " (" << r.to_string() << ")";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

double Network::tn_path_size(glp_graph *tn, const ivector &path) {
    glp_vertex *v, *w;
    glp_arc *a;
    int n = path.size();
    double ret = 0;
    for (int i = 0; i < n - 1; ++i) {
        v = tn->v[path[i]];
        w = tn->v[path[i+1]];
        a = v->out;
        while (a != NULL) {
            if (a->head == w) break;
            a = a->t_next;
        }
        assert(a != NULL);
        ret += t_adata(a)->weight;
    }
    return ret;
}

ivector Network::dijkstra(glp_graph *tn, int src, int dest) {
    assert(src > 0 && src <= tn->nv && (dest == 0 || dest <= tn->nv));
    int n = tn->nv, i;
    ivector Q;
    Q.reserve(n);
    std::vector<bool> popped(n + 1, false);
    glp_vertex *u, *v;
    glp_arc *a;
    double mindist, alt;
    for (i = 1; i <= n; ++i) {
        v = tn->v[i];
        t_vdata(v)->dist = i == src ? 0 : DBL_MAX;
        t_vdata(v)->parent = 0;
        Q.push_back(i);
    }
    while (!Q.empty()) {
        u = NULL;
        for (ivector::const_iterator it = Q.begin(); it!=Q.end(); ++it) {
            if (u == NULL || t_vdata(tn->v[*it])->dist < mindist) {
                u = tn->v[*it];
                mindist = t_vdata(u)->dist;
                n = (int)(it - Q.begin());
            }
        }
        assert(u != NULL);
        Q.erase(Q.begin() + n);
        if (u->i == dest) {
            ivector path;
            i = dest;
            while(i > 0) {
                path.push_back(i);
                i = t_vdata(tn->v[i])->parent;
            }
            if (path.back() != src)  return ivector(0);
            std::reverse(path.begin(), path.end());
            return path;
        }
        popped[u->i] = true;
        a = u->out;
        while (a != NULL) {
            v = a->head;
            if (!popped[v->i]) {
                alt = t_vdata(u)->dist + t_adata(a)->weight;
                if (alt < t_vdata(v)->dist) {
                    t_vdata(v)->dist = alt;
                    t_vdata(v)->parent = u->i;
                }
            }
            a = a->t_next;
        }
    }
    return ivector(0);
}
