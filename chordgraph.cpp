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
 * Septima is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Septima.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "chordgraph.h"
#include "transitionnetwork.h"
#include <assert.h>
#include <math.h>

ChordGraph::ChordGraph(const std::vector<Chord> &chords, int k, const Domain &sup, PreparationScheme p, bool aug, bool use_labels,
                       bool is_weighted, bool dot_tex) :
    Digraph(is_weighted, dot_tex)
{
    int i, j;
    _support = sup;
    M = k;
    _allows_aug = aug;
    for (std::vector<Chord>::const_iterator it = chords.begin(); it != chords.end(); ++it) {
        if (Realization::tonal_realizations(*it, sup, aug).empty())
            continue;
        i = add_vertices(1);
        if (use_labels)
            set_vertex_name(i, (dot_tex ? it->to_tex() : it->to_string()).c_str());
        chord_map[i] = *it;
    }
    std::set<Transition> bt;
    int n = number_of_vertices();
    for (i = 1; i <= n; ++i) {
        const Chord &c1 = chord_map[i];
        for (j = 1; j <= n; ++j) {
            if (i == j) continue;
            const Chord &c2 = chord_map[j];
            bt = Transition::elementary_transitions(c1, c2, k, sup, p, aug);
            if (bt.empty())
                continue;
            glp_arc *a = add_arc(i, j);
            transition_map[a] = bt;
        }
    }
}

int ChordGraph::class_index() const {
    return M;
}

const Domain &ChordGraph::support() const {
    return _support;
}

bool ChordGraph::allows_augmented_sixths() const {
    return _allows_aug;
}

const std::set<Transition> &ChordGraph::transitions(glp_arc *a) const {
    return transition_map.at(a);
}

int ChordGraph::find_vertex_by_chord(const Chord &c) const {
    for (int i = 1; i <= number_of_vertices(); ++i) {
        if (chord_map.at(i) == c)
            return i;
    }
    return 0;
}

const Chord &ChordGraph::vertex2chord(int i) const {
    assert (i > 0 && i <= number_of_vertices());
    return chord_map.at(i);
}

bool ChordGraph::best_voicing(const std::vector<Chord> &seq, int &z0, double spread_weight, double vl_weight, double aug_weight, voicing &v) const {
    ivector walk;
    for (std::vector<Chord>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
        int v = find_vertex_by_chord(*it);
        if (v == 0 || (it != seq.begin() && arc(walk.back(), v) == NULL))
            return false;
        walk.push_back(v);
    }
    std::vector<double> wgh;
    wgh.push_back(spread_weight);
    wgh.push_back(vl_weight);
    wgh.push_back(aug_weight);
    z0 = TransitionNetwork::optimal_voicing(*this, walk, wgh, v);
    return true;
}

bool ChordGraph::best_voicings(const std::vector<Chord> &seq, double spread_weight, double vl_weight, double aug_weight, std::set<voicing> &vs) const {
    ivector walk;
    for (std::vector<Chord>::const_iterator it = seq.begin(); it != seq.end(); ++it) {
        int v = find_vertex_by_chord(*it);
        if (v == 0 || (it != seq.begin() && arc(walk.back(), v) == NULL))
            return false;
        walk.push_back(v);
    }
    std::vector<double> wgh;
    wgh.push_back(spread_weight);
    wgh.push_back(vl_weight);
    wgh.push_back(aug_weight);
    vs = TransitionNetwork::all_optimal_voicings(*this, walk, wgh);
    return true;
}

void ChordGraph::shortest_paths(int src, int dest, std::vector<ivector> &paths) {
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

void ChordGraph::all_shortest_paths(pathmap &path_map) {
    int n = number_of_vertices();
    path_map.clear();
    int total_paths = 0, total_length = 0, len;
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
            }
        }
    }
}

double ChordGraph::betweenness_centrality(int i, const pathmap &path_map) const {
    int n = number_of_vertices(), m, tot = 0, feas = 0;
    for (int j = 1; j <= n; ++j) {
        if (i == j)
            continue;
        for (int k = 1; k <= n; ++k){
            if (j == k || k == i)
                continue;
            const std::vector<ivector> &paths = path_map.at(std::make_pair(j, k));
            tot += paths.size();
            m = 0;
            for (std::vector<ivector>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
                if (std::find(it->begin(), it->end(), i) != it->end())
                    ++m;
            }
            feas += m;
        }
    }
    return double(feas) / double(tot);
}

double::ChordGraph::communicability_betweenness_centrality(int k) const {
    int n = number_of_vertices();
    if (n < 3)
        return 0;
    Matrix A = adjacency_matrix(), Ak = A;
    for (int i = 1; i <= n; ++i) {
        Ak.set_element(i, k, 0.0);
    }
    Matrix eA = A.exponential(), eAk = Ak.exponential();
    double ret = 0;
    for (int i = 1; i <= n; ++i) {
        if (i == k)
            continue;
        for (int j = 1; j <= n; ++j) {
            if (j == k || j == i)
                continue;
            ret += 1 - eAk.element(i, j) / eA.element(i, j);
        }
    }
    return ret / double((n - 1) * (n - 2));
}

double ChordGraph::katz_centrality(int k, bool rev) const {
    Matrix A = adjacency_matrix();
    double max_eigval = -1, d;
    std::vector<std::pair<double,double> > ev = A.eigenvalues();
    for (std::vector<std::pair<double,double> >::const_iterator it = ev.begin(); it != ev.end(); ++it) {
        d = sqrt(it->first * it->first + it->second * it->second);
        if (d > max_eigval)
            max_eigval = d;
    }
    if (d == 0)
        return DBL_MAX;
    double r = 1.0 / d, frac = 0.9;
    double lambda = r * frac;
    A.scale(-lambda);
    Matrix B = Matrix::identity(A.size());
    B.add(A);
    Matrix I = B.inverse();
    double ret = 0.0;
    for (int i = 1; i <= I.size(); ++i) {
        ret += rev ? I.element(i, k) : I.element(k, i);
    }
    return ret;
}

double ChordGraph::closeness_centrality(int i) {
    int n = number_of_vertices(), d = 0;
    ivector path;
    for (int j = 1; j <= n; ++j) {
        if (i == j)
            continue;
        bfs(i, j, path);
        d += path.size() - 1;
    }
    return (double)(n - 1)/(double)d;
}

int ChordGraph::rand_int(int n) {
    return rand() % n;
}

ivector ChordGraph::rand_perm(int n) {
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

void ChordGraph::make_acyclic(const ivector &perm) {
    glp_vertex *v;
    glp_arc *a;
    int i, j;
    for (i = 1; i <= number_of_vertices(); ++i) {
        v = vertex(i);
        a = v->out;
        while (a != NULL) {
            j = a->head->i;
            if (perm[i] > perm[j])
                arc_data(a)->active = false;
            a = a->t_next;
        }
    }
}

void ChordGraph::find_fixed_length_paths(int src, int dest, int len, int limit, std::vector<ivector> &paths) {
    int nv = number_of_vertices();
    assert(len > 1 && src > 0 && src <= nv && dest > 0 && dest <= nv);
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
                p = rand_perm(number_of_vertices());
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
        }
        if (perm_set.empty()) continue;
        const std::pair<double, ivector> &bp = *perm_set.begin();
        s -= bp.second.back();
        make_acyclic(bp.second);
        std::vector<ivector> yp;
        yen(src, dest, 0, len, len, yp);
        for (std::vector<ivector>::const_iterator it = yp.begin(); it != yp.end(); ++it) {
            path_set.insert(*it);
            if ((int)path_set.size() == limit) break;
        }
        perm_set.erase(perm_set.begin());
        enable_all_arcs();
    }
    for (std::set<ivector, path_comp>::const_iterator it = path_set.begin(); it != path_set.end(); ++it) {
        paths.push_back(*it);
        if ((int)paths.size() == limit) break;
    }
}
