/* transitionnetwork.cpp
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

#include "transitionnetwork.h"
#include <assert.h>
#include <float.h>

TransitionNetwork::TransitionNetwork(const ChordGraph &cg, const ivector &walk, const Realization &r, const std::vector<double> &wgh, int z,
                                     bool verbose) :
    Digraph(true, false, verbose)
{
    X0 = r;
    nl = walk.size() - 1;
    M = cg.class_index();
    _num_paths = 1;
    glp_vertex *v, *w;
    glp_arc *a;
    int mc, l, vi, i, j, tcn, d = 7;
    ivector f;
    std::map<int,ivector> levels;
    /* create vertices, arranged in levels */
    for (l = nl; l-->0;) {
        i = walk[l];
        j = walk[l+1];
        a = cg.arc(i, j);
        assert(a != NULL);
        const std::set<Transition> &ta = cg.transitions(a);
        vi = add_vertices(ta.size());
        _num_paths *= ta.size();
        for (std::set<Transition>::const_iterator it = ta.begin(); it != ta.end(); ++it) {
            transition_map[vi] = &(*it);
            levels[l+1].push_back(vi);
            ++vi;
        }
    }
    /* create edges between levels */
    for (l = 1; l < nl; ++l) {
        ivector &lev1 = levels[l], &lev2 = levels[l+1];
        for (ivector::const_iterator it = lev1.begin(); it != lev1.end(); ++it) {
            v = vertex(*it);
            const Transition &t1 = *(transition_map.at(*it));
            for (ivector::const_iterator jt = lev2.begin(); jt != lev2.end(); ++jt) {
                w = vertex(*jt);
                const Transition &t2 = *(transition_map.at(*jt));
                assert(t2.glue(t1.second(), mc, tcn, f, cg.class_index()));
                a = add_arc(v->i, w->i);
                phi_map[a] = f;
                cues_map[a] = mc > 0;
                /* compute the arc weight */
                double wg = wgh[0] * t2.second().lof_point_distance(z) / d + sqrt(tcn / 4) * wgh[1] / M;
                if (t2.second().is_augmented_sixth())
                    wg += wgh[2];
                if (l == 1) {
                    assert(t1.glue(X0, mc, tcn, f));
                    wg += wgh[0] * X0.lof_point_distance(z) / d;
                    wg += wgh[0] * t1.second().lof_point_distance(z) / d + sqrt(tcn / 4) * wgh[1] / M;
                    if (t1.second().is_augmented_sixth())
                        wg += wgh[2];
                    if (X0.is_augmented_sixth())
                        wg += wgh[2];
                }
                arc_data(a)->weight = wg;
            }
        }
    }
    /* get sources and sinks */
    for (int i = 1; i <= number_of_vertices(); ++i) {
        v = vertex(i);
        if (v->in == NULL)
            _sources.push_back(i);
        else if (v->out == NULL)
            _sinks.push_back(i);
    }
}

const ivector &TransitionNetwork::sources() const {
    return _sources;
}

const ivector &TransitionNetwork::sinks() const {
    return _sinks;
}

int TransitionNetwork::num_levels() const {
    return nl;
}

int TransitionNetwork::num_paths() const {
    return _num_paths;
}

ivector TransitionNetwork::best_path() {
    ivector bp, p;
    double w, min_weight = DBL_MAX;
    enable_all_arcs();
    enable_all_vertices();
    for (ivector::const_iterator it = _sources.begin(); it != _sources.end(); ++it) {
        dijkstra(*it);
        for (ivector::const_iterator jt = _sinks.begin(); jt != _sinks.end(); ++jt) {
            p = get_path(*jt);
            w = path_weight(p);
            if (w < min_weight) {
                bp = p;
                min_weight = w;
            }
        }
    }
    return bp;
}

std::vector<ivector> TransitionNetwork::best_paths(double &theta) {
    ivector bp = best_path();
    theta = path_weight(bp);
    std::vector<ivector> paths, ret;
    for (ivector::const_iterator it = _sources.begin(); it != _sources.end(); ++it) {
        for (ivector::const_iterator jt = _sinks.begin(); jt != _sinks.end(); ++jt) {
            enable_all_arcs();
            enable_all_vertices();
            yen(*it, *jt, 0, theta, theta, paths);
            ret.insert(ret.end(), paths.begin(), paths.end());
        }
    }
    return ret;
}

pitchSpelling TransitionNetwork::realize_path(const ivector &path) {
    glp_vertex *v, *w;
    glp_arc *a;
    int n = path.size(), mc, tcn;
    ivector f(4), f0;
    pitchSpelling ret;
    for (int i = 0; i < n; ++i) {
        v = vertex(path[i]);
        const Transition &t = *(transition_map.at(v->i));
        Realization r1 = t.first(), r2 = t.second();
        if (i == 0)
            assert(t.glue(X0, mc, tcn, f, M));
        r1.arrange(f);
        r2.arrange(f);
        if (i == 0) {
            ret.push_back(std::make_pair(X0, false));
            if (mc > 0)
                ret.push_back(std::make_pair(r1, true));
        }
        ret.push_back(std::make_pair(r2, false));
        if (i != n - 1) {
            w = vertex(path[i+1]);
            a = v->out;
            while (a != NULL) {
                if (a->head == w) break;
                a = a->t_next;
            }
            assert(a != NULL);
            f = compose(f, phi_map.at(a));
            if (cues_map.at(a)) {
                const Transition &u = *(transition_map.at(w->i));
                Realization r = u.first();
                r.arrange(f);
                ret.push_back(std::make_pair(r, true));
            }
        }
    }
    return ret;
}

ivector TransitionNetwork::compose(const ivector &f1, const ivector &f2) {
    assert(f1.size() == 4 && f2.size() == 4);
    ivector tmp(4);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = f2[f1[i]];
    }
    return tmp;
}

double TransitionNetwork::cog_offset_factor(int z) {
    return 1.0; // + (z * z) / 50.0;
}

int TransitionNetwork::optimal_pitch_spelling(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh, pitchSpelling &ps) {
    const Chord &c0 = cg.vertex2chord(walk.front());
    Domain dom = cg.support();
    double w, min_w = DBL_MAX;
    int best_z;
    std::vector<Realization> R = Realization::tonal_realizations(c0, dom, cg.allows_augmented_sixths());
    for (std::vector<Realization>::const_iterator it = R.begin(); it != R.end(); ++it) {
        for (int z = dom.lbound(); z <= dom.ubound(); ++z) {
            TransitionNetwork tn(cg, walk, *it, wgh, z);
            ivector bp = tn.best_path();
            w = tn.path_weight(bp) * cog_offset_factor(z);
            if (w < min_w) {
                ps = tn.realize_path(bp);
                min_w = w;
                best_z = z;
            }
        }
    }
    arrange_voices(ps);
    return best_z;
}

bool TransitionNetwork::are_pitch_spellings_equivalent(const pitchSpelling &ps1, const pitchSpelling &ps2) {
    assert(!ps1.empty() && !ps2.empty());
    if (ps1.size() != ps2.size())
        return false;
    int n = ps1.size(), d;
    for (int i = 0; i < n; ++i) {
        if (ps1.at(i).second != ps2.at(i).second)
            return false;
        const Realization &r1 = ps1.at(i).first;
        Realization r2 = ps2.at(i).first;
        if (i == 0) {
            d = r1.tone_set().begin()->lof_position() - r2.tone_set().begin()->lof_position();
            if (d % 12)
                return false;
        }
        r2.transpose(d);
        if (r1.tone_set() != r2.tone_set())
            return false;
    }
    return true;
}

std::set<pitchSpelling> TransitionNetwork::all_optimal_pitch_spellings(const ChordGraph &cg, const ivector &walk, const std::vector<double> &wgh) {
    const Chord &c0 = cg.vertex2chord(walk.front());
    Domain dom = cg.support();
    double theta;
    std::set<pitchSpelling> res;
    std::set<std::pair<int,pitchSpelling> > best_ps;
    std::set<std::pair<std::pair<double,int>,pitchSpelling> > all_ps;
    std::vector<Realization> R = Realization::tonal_realizations(c0, dom, cg.allows_augmented_sixths());
    for (std::vector<Realization>::const_iterator it = R.begin(); it != R.end(); ++it) {
        for (int z = dom.lbound(); z <= dom.ubound(); ++z) {
            TransitionNetwork tn(cg, walk, *it, wgh, z);
            std::vector<ivector> bpv = tn.best_paths(theta);
            theta *= cog_offset_factor(z);
            for (std::vector<ivector>::const_iterator jt = bpv.begin(); jt != bpv.end(); ++jt) {
                all_ps.insert(std::make_pair(std::make_pair(theta, z), tn.realize_path(*jt)));
            }
        }
    }
    if (!all_ps.empty()) {
        double theta0 = all_ps.begin()->first.first;
        for (std::set<std::pair<std::pair<double,int>,pitchSpelling> >::const_iterator it = all_ps.begin(); it != all_ps.end(); ++it) {
            if (it->first.first <= theta0)
                best_ps.insert(std::make_pair(it->first.second, it->second));
        }
        assert(!best_ps.empty());
        bool found;
        std::set<std::pair<int,pitchSpelling> >::const_iterator it, jt;
        while (true) {
            found = false;
            for (it = best_ps.begin(); it != best_ps.end(); ++it) {
                for (jt = best_ps.begin(); jt != best_ps.end(); ++jt) {
                    if (it != jt && are_pitch_spellings_equivalent(it->second, jt->second)) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }
            if (found) {
                if (abs(it->first) < abs(jt->first))
                    best_ps.erase(jt);
                else best_ps.erase(it);
            } else break;
        }
        for (it = best_ps.begin(); it != best_ps.end(); ++it) {
            pitchSpelling ps = it->second;
            arrange_voices(ps);
            res.insert(ps);
        }
    }
    return res;
}

void TransitionNetwork::arrange_voices(pitchSpelling &ps) {
    int len = ps.size(), cnt;
    std::vector<std::pair<int,pitchSpelling> > ind;
    ivector F(4);
    for (int i = 0; i < 24; ++i) {
        pitchSpelling chn = ps;
        const int *f = Transition::sym4[i];
        for (int k = 0; k < 4; ++k) F[k] = f[k];
        cnt = 0;
        for (int j = 0; j < len; ++j) {
            Realization &r = chn[j].first;
            r.arrange(F);
            if (j > 0) {
                Realization &pred = chn[j-1].first;
                for (int l = 0; l < 3; ++l) {
                    for (int u = l + 1; u < 4; ++u) {
                        if (pred.tone(l).pitch_class() != r.tone(l).pitch_class() &&
                                pred.tone(l).interval(pred.tone(u)).second == 7 &&
                                r.tone(l).interval(r.tone(u)).second == 7)
                            ++cnt;
                    }
                }
            }
        }
        ind.push_back(std::make_pair(cnt, chn));
    }
    std::sort(ind.begin(), ind.end());
    int min_cnt = ind.front().first, min_gs = RAND_MAX, gs, min_ss = RAND_MAX, ss;
    for (std::vector<std::pair<int,pitchSpelling> >::const_iterator it = ind.begin(); it != ind.end(); ++it) {
        if (it->first > min_cnt)
            break;
        gs = ss = 0;
        for (pitchSpelling::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
            if (jt->second)
                continue;
            for (int i = 0; i < 3; ++i) {
                ipair I = jt->first.tone(i).interval(jt->first.tone(i+1));
                gs += I.first;
                ss += I.second;
            }
        }
        if (gs < min_gs || (gs == min_gs && ss < min_ss)) {
            ps = it->second;
            min_ss = ss;
            if (gs < min_gs)
                min_gs = gs;
        }
    }
}

std::ostream& operator <<(std::ostream &os, const pitchSpelling &ps) {
    for (pitchSpelling::const_iterator it = ps.begin(); it != ps.end(); ++it) {
        if (it->second)
            os << "(";
        os << it->first;
        if (it->second)
            os << ")";
        os << std::endl;
    }
    return os;
}
