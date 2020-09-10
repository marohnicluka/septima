/* transition.cpp
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

#include "transition.h"
#include <assert.h>
#include <map>
#include <cmath>
#include <sstream>

Transition::Transition(const Realization &a, const Realization &b) {
    _first = a;
    _second = b;
}

Transition::Transition(const Transition &other) {
    _first = other.first();
    _second = other.second();
}

Transition& Transition::operator =(const Transition &other) {
    _first = other.first();
    _second = other.second();
    return *this;
}

void Transition::set_first(const Realization &r) {
    _first = r;
}

void Transition::set_second(const Realization &r) {
    _second = r;
}

const Realization& Transition::first() const {
    return _first;
}

const Realization& Transition::second() const {
    return _second;
}

double Transition::MAD(int z) const {
    std::set<Tone> X = first().tone_set(), Y = second().tone_set();
    std::set<Tone>::const_iterator it;
    for (it = Y.begin(); it != Y.end(); ++it) {
        X.insert(*it);
    }
    double sum = 0;
    for (it = X.begin(); it != X.end(); ++it) {
        sum += abs(it->lof_position() - z);
    }
    return sum / double(X.size());
}

double Transition::lof_distance(int z, bool maximum) const {
    std::set<Tone> X = first().tone_set(), Y = second().tone_set();
    std::set<Tone>::const_iterator it;
    for (it = Y.begin(); it != Y.end(); ++it) {
        X.insert(*it);
    }
    double dist = maximum ? 0 : RAND_MAX;
    for (it = X.begin(); it != X.end(); ++it) {
        int d = abs(it->lof_position() - z);
        if ((maximum && d > dist) || (!maximum && d < dist))
            dist = d;
    }
    return dist;
}

int Transition::diameter() const {
    std::set<Tone> st = this->tone_set();
    return Tone::lof_distance(*st.begin(), *st.rbegin());
}

std::set<Tone> Transition::tone_set() const {
    std::set<Tone> s1 = first().tone_set(), s2 = second().tone_set();
    for (std::set<Tone>::const_iterator it = s2.begin(); it != s2.end(); ++it) {
        s1.insert(*it);
    }
    return s1;
}

bool Transition::is_closer_than(const Transition &other, int z) const {
    double lps1 = this->MAD(z), lps2 = other.MAD(z);
    if (lps1 == lps2) {
        int m1 = 0, m2 = 0;
        for (int i = 0; i < 4; ++i) {
            m1 += abs(this->first().tone(i).lof_position() + this->second().tone(i).lof_position() - 2 * z);
            m2 = abs(other.first().tone(i).lof_position() + other.second().tone(i).lof_position() - 2 * z);
        }
        return m1 < m2;
    }
    return lps1 < lps2;
}

bool Transition::operator ==(const Transition &other) const {
    std::map<Tone,Tone> vl1, vl2;
    for (int i = 0; i < 4; ++i) {
        vl1[this->first().tone(i)] = this->second().tone(i);
        vl2[other.first().tone(i)] = other.second().tone(i);
    }
    return vl1 == vl2;
}

bool Transition::operator !=(const Transition &other) const {
    return !(*this == other);
}

/* Kochavi (2008) measure of parsimony (line-of-fifths spreads are compared in case of equal degrees of parsimony) */
bool Transition::operator <(const Transition &other) const {
    int ct1 = this->common_pc_count(), ct2 = other.common_pc_count();
    if (ct1 == ct2) {
        int vl1 = this->vl_shift(), vl2 = other.vl_shift();
        if (vl1 == vl2) {
            int dvl1 = this->directional_vl_shift(), dvl2 = other.directional_vl_shift();
            if (dvl1 == dvl2) {
                double ls1 = this->lof_spread(), ls2 = other.lof_spread();
                if (ls1 == ls2) {
                    double vls1 = this->vl_lof_spread(), vls2 = other.vl_lof_spread();
                    if (vls1 == vls2) {
                        if (this->first() == other.first())
                            return this->second() < other.second();
                        return this->first() < other.first();
                    }
                    return vls1 < vls2;
                }
                return ls1 < ls2;
            }
            return dvl1 < dvl2;
        }
        return vl1 < vl2;
    }
    return ct1 > ct2;
}

bool Transition::glue(const Realization &pred, int &mc, int &tcn, std::vector<int> &f, int k) const {
    if (!pred.is_enharmonically_equal(_first))
        return false;
    f = std::vector<int>(4, -1);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (pred.tone(i).pitch_class() == _first.tone(j).pitch_class()) {
                f[i] = j;
                break;
            }
        }
    }
    mc = tcn = 0;
    bool r = true;
    for (int i = 0; i < 4; ++i) {
        int d = Tone::lof_distance(pred.tone(i), _second.tone(f[i]));
        if (r && d > k)
            r = false;
        tcn += d * d;
        if (pred.tone(i).note_name() != _first.tone(f[i]).note_name())
            ++mc;
    }
    if (r)
        mc = 0;
    return true;
}

bool Transition::is_enharmonically_equal(const Transition &other) const {
    std::map<int,int> vl1, vl2;
    for (int i = 0; i < 4; ++i) {
        vl1[this->first().tone(i).pitch_class()] = this->second().tone(i).pitch_class();
        vl2[other.first().tone(i).pitch_class()] = other.second().tone(i).pitch_class();
    }
    return vl1 == vl2;
}

bool Transition::is_congruent(const Transition &other) const {
    return is_structurally_equal(other, true);
}

bool Transition::is_structurally_equal(const Transition &other, bool enharm) const {
    std::set<Tone> X1 = this->tone_set(), X2 = other.tone_set();
    int d = X1.begin()->lof_position() - X2.begin()->lof_position();
    if (enharm && d % 12)
        return false;
    Realization r1(other.first()), r2(other.second());
    for (int i = 0; i < 4; ++i) {
        r1.tone(i).transpose(d);
        r2.tone(i).transpose(d);
    }
    return Transition(r1, r2) == *this;
}

Transition Transition::structural_inversion() const {
    Transition ret(this->first().structural_inverse(), this->second().structural_inverse());
    return ret;
}

Transition Transition::retrograde() const {
    Transition ret(this->second(), this->first());
    return ret;
}

bool Transition::is_equivalent_up_to_transposition_and_rotation(const Transition &other) const {
    Transition t1 = this->structural_inversion(), t2 = this->retrograde(), t3 = t1.retrograde();
    return this->is_structurally_equal(other) || t1.is_structurally_equal(other) || t2.is_structurally_equal(other) || t3.is_structurally_equal(other);
}

bool Transition::is_smooth() const {
    for (int i = 0; i < 4; ++i) {
        if (Tone::interval_abs(first().tone(i), second().tone(i)).second > 2)
            return false;
    }
    return true;
}

bool Transition::is_efficient() const {
    int d = first().chord().vl_efficiency_metric(second().chord());
    return vl_shift() <= d;
}

std::string Transition::to_string() const {
    std::string s1 = _first.to_string(), s2 = _second.to_string();
    return s1 + " -> " + s2;
}

const char* Transition::chord_type_names[] = {
    "d7", "&O;", "m7", "\\triangle ##f", "o7", "Ger", "TC"
};

std::string Transition::to_lily(int mp, int prep, bool ch) const {
    std::vector<int> P, Q, R(9), S(4), T(4);
    notated_chord_compare comp(mp);
    std::vector<std::vector<int> > nt;
    for (int i = 0; i < 4; ++i) {
        P.push_back(first().tone(i).pitch_class());
        Q.push_back(second().tone(i).pitch_class());
    }
    for (int i = 0; i < 4; ++i) {
        int dir = (Q[i] < P[i] ? 1 : -1);
        while (abs(Q[i] - P[i]) >= 6) {
            Q[i] += 12 * dir;
        }
    }
    bool fifths;
    for (int inv = 0; inv < 23; ++inv) {
        for (int i = 0; i < 4; ++i) {
            S[i] = P[sym4[inv][i]];
            T[i] = Q[sym4[inv][i]];
        }
        R[0] = inv;
        for (int n = 0; n < 12; ++n) {
            for (int i = -2; i <= 2; ++i) {
                for (int j = -2; j <= 2; ++j) {
                    for (int k = -2; k <= 2; ++k) {
                        R[1] = S[0] + n * 12;       R[5] = T[0] + n * 12;
                        R[2] = S[1] + (n + i) * 12; R[6] = T[1] + (n + i) * 12;
                        R[3] = S[2] + (n + j) * 12; R[7] = T[2] + (n + j) * 12;
                        R[4] = S[3] + (n + k) * 12; R[8] = T[3] + (n + k) * 12;
                        if (R[1] > R[2] || R[2] > R[3] || R[3] > R[4] || R[5] > R[6] || R[6] > R[7] || R[7] > R[8])
                            continue;
                        fifths = false;
                        for (int m1 = 1; !fifths && m1 <= 3; ++m1) {
                            for (int m2 = m1 + 1; m2 <= 4; ++m2) {
                                if ((R[m2] - R[m2+4]) * (R[m1] - R[m1+4]) > 0 && R[m2+4] - R[m1+4] == 7) {
                                    fifths = true;
                                    break;
                                }
                            }
                        }
                        if (!fifths)
                            nt.push_back(R);
                    }
                }
            }
        }
    }
    std::sort(nt.begin(), nt.end(), comp);
    const std::vector<int> &r0 = nt.front();
    int inv = r0.front(), a, b, oct;
    const int diat[] = { 0, 2, 4, 5, 7, 9, 11 };
    std::vector<int> oct1(4), oct2(4);
    for (int i = 0; i < 4; ++i) {
        const Tone &t1 = first().tone(sym4[inv][i]), &t2 = second().tone(sym4[inv][i]);
        a = diat[t1.note_name()] + t1.accidental() - 60;
        b = r0[i+1];
        oct = -9;
        while (a < b) {
            a += 12;
            ++oct;
        }
        oct1[i] = oct;
        a = diat[t2.note_name()] + t2.accidental() - 60;
        b = r0[i+5];
        oct = -9;
        while (a < b) {
            a += 12;
            ++oct;
        }
        oct2[i] = oct;
    }
    int svg = second().generic_seventh_voice(), sv;
    for (sv = 0; sv < 4 && sym4[inv][sv] != svg; ++sv);
    std::stringstream ss;
    ss << "<";
    for (int i = 0; i < 4; ++i) {
        if (prep > 0 && i == sv)
            ss << "\\tweak duration-log #2 ";
        ss << first().tone(sym4[inv][i]).to_lily();
        oct = oct1[i];
        for (int j = 0; j < abs(oct); ++j) {
            ss << (oct < 0 ? "," : "'");
        }
        ss << (i < 3 ? " " : ">1");
    }
    if (prep == 2)
        ss << "(";
    if (ch)
        ss << "^\\markup\\sans{" << chord_type_names[first().type()] << "}";
    ss << " <";
    for (int i = 0; i < 4; ++i) {
        if (prep && i == sv)
            ss << "\\tweak duration-log #2 ";
        ss << second().tone(sym4[inv][i]).to_lily();
        oct = oct2[i];
        for (int j = 0; j < abs(oct); ++j) {
            ss << (oct < 0 ? "," : "'");
        }
        ss << (i < 3 ? " " : ">1");
    }
    if (prep == 2)
        ss << ")";
    if (ch)
        ss << "^\\markup\\sans{" << chord_type_names[second().type()] << "}";
    return ss.str();
}

double Transition::lof_spread() const {
    std::set<Tone> X = first().tone_set(), Y = second().tone_set();
    std::set<Tone>::const_iterator it;
    for (it = Y.begin(); it != Y.end(); ++it) {
        X.insert(*it);
    }
    double mean = 0, var = 0;
    for(it = X.begin(); it != X.end(); ++it) {
        mean += it->lof_position();
    }
    mean /= double(X.size());
    for (it = X.begin(); it != X.end(); ++it) {
        var += (mean - it->lof_position()) * (mean - it->lof_position());
    }
    var /= double(X.size());
    return sqrt(var);
}

double Transition::vl_lof_spread() const {
    double sum = 0.0;
    for (int i = 0; i < 4; ++i) {
        double t = double(first().tone(i).lof_position() + second().tone(i).lof_position()) / 2.0;
        sum += t * t;
    }
    return sqrt(sum / 4.0);
}

int Transition::augmented_count(bool tristan) const {
    int cnt = 0;
    if (first().is_augmented_sixth(tristan))
        ++cnt;
    if (second().is_augmented_sixth(tristan))
        ++cnt;
    return cnt;
}

int Transition::vl_shift() const {
    int pd = 0;
    for (int i = 0; i < 4; ++i) {
        pd += Tone::modd(first().tone(i).pitch_class() - second().tone(i).pitch_class(), 12);
    }
    return pd;
}

int Transition::lof_shift() const {
    int pd = 0;
    for (int i = 0; i < 4; ++i) {
        pd += Tone::lof_distance(first().tone(i), second().tone(i));
    }
    return pd;
}

int Transition::directional_vl_shift() const {
    int pd = 0, d;
    for (int i = 0; i < 4; ++i) {
        d = Tone::modb(second().tone(i).pitch_class() - first().tone(i).pitch_class(), 12);
        if (d > 6)
            d -= 12;
        pd += d;
    }
    return abs(pd);
}

int Transition::directional_lof_shift() const {
    int pd = 0;
    for (int i = 0; i < 4; ++i) {
        pd += second().tone(i).lof_position() - first().tone(i).lof_position();
    }
    return abs(pd);
}

int Transition::degree() const {
    int norm = 0, d;
    for (int i = 0; i < 4; ++i) {
        d = Tone::lof_distance(first().tone(i), second().tone(i));
        if (d > norm)
            norm = d;
    }
    return norm;
}

int Transition::common_pc_count() const {
    std::set<Tone> ts1 = first().tone_set(), ts2 = second().tone_set();
    assert(ts1.size() == 4 && ts2.size() == 4);
    for (std::set<Tone>::const_iterator it = ts2.begin(); it != ts2.end(); ++it) {
        ts1.insert(*it);
    }
    return 8 - int(ts1.size());
}

bool Transition::acts_identically_on_pc_intersection() const {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j)
                continue;
            if (first().tone(i).pitch_class() == second().tone(j).pitch_class())
                return false;
        }
    }
    return true;
}

ipair Transition::mn_type() const {
    int s = 0, w = 0, d = degree();
    for (int i = 0; i < 4; ++i) {
        ipair p = Tone::interval_abs(first().tone(i), second().tone(i));
        if (p.second == 1)
            ++s;
        else if (p.second == 2)
            ++w;
        else if (d <= 7 && p.second > 2)
            assert(false);
    }
    return std::make_pair(s, w);
}

bool Transition::is_prepared_generic() const {
    int sv = second().generic_seventh_voice();
    return first().tone(sv) == second().tone(sv);
}

void Transition::transpose(int d) {
    _first.transpose(d);
    _second.transpose(d);
}

/* the symmetric group of order four */
const int Transition::sym4[][4] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
    {0, 3, 1, 2}, {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2},
    {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
    {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0},
    {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 0, 2, 1},
    {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
};

std::set<Transition> Transition::elementary_transitions(const Chord &c1, const Chord &c2, int k, const Domain &dom, PreparationScheme p, bool aug) {
    std::vector<Realization> br1 = Realization::tonal_realizations(c1, dom, aug);
    std::vector<Realization> br2 = Realization::tonal_realizations(c2, dom, aug);
    std::set<Transition> ret;
    std::vector<int> f(4, -1);
    int rv1, rv2, sv, i, j, s;
    for (std::vector<Realization>::const_iterator it = br1.begin(); it != br1.end(); ++it) {
        rv1 = it->generic_root_voice();
        for (std::vector<Realization>::const_iterator jt = br2.begin(); jt != br2.end(); ++jt) {
            rv2 = jt->generic_root_voice();
            for (i = 0; i < 24; ++i) {
                s = 0;
                for (j = 0; j < 4; ++j) {
                    f[j] = sym4[i][j];
                    if (Tone::lof_distance(it->tone(j), jt->tone(f[j])) > k)
                        break;
                    else s += Tone::modd(3 * Tone::lof_distance(it->tone(j), jt->tone(f[j])), 7);
                }
                if (j == 4 && s == Tone::modd(2 * Tone::lof_distance(it->tone(rv1), jt->tone(rv2)), 7)) {
                    Realization r1(*it), r2(*jt);
                    r2.arrange(f);
                    Transition T(r1, r2);
                    if (T.lof_shift() == 28)
                        continue;
                    if (p == PREPARE_GENERIC) {
                        sv = r2.generic_seventh_voice();
                        if (r1.tone(sv) == r2.tone(sv))
                            ret.insert(T);
                    } else {
                        sv = r2.acoustic_seventh_voice();
                        if (p == NO_PREPARATION || sv < 0 || (p == PREPARE_ACOUSTIC_NO_DOMINANT && c2.type() == DOMINANT_SEVENTH) ||
                                r1.tone(sv).pitch_class() == r2.tone(sv).pitch_class())
                            ret.insert(T);
                    }
                }
            }
        }
    }
    return ret;
}

std::vector<Transition> Transition::elementary_classes(const Chord &c1, const Chord &c2, int k, PreparationScheme p, int z, bool aug) {
    int r = 11 + k/2;
    Domain dom;
    dom.insert_range(z-r, z+r);
    std::set<Transition> E = elementary_transitions(c1, c2, k, dom, p, aug);
    std::vector<Transition> ret;
    bool found;
    for (std::set<Transition>::const_iterator it = E.begin(); it != E.end(); ++it) {
        found = false;
        std::vector<Transition>::const_iterator jt;
        for (jt = ret.begin(); jt != ret.end(); ++jt) {
            if (it->is_congruent(*jt)) {
                found = true;
                break;
            }
        }
        if (found) {
            if (it->is_closer_than(*jt, z)) {
                ret.erase(jt);
                ret.push_back(*it);
            }
        } else ret.push_back(*it);
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

std::vector<Transition> Transition::elementary_types(const std::vector<Chord> &chords, int k, PreparationScheme p, int z, bool aug, bool simp) {
    std::vector<Chord>::const_iterator it, jt;
    std::vector<Transition>::const_iterator kt, st;
    std::vector<Transition> cls;
    std::set<std::pair<std::vector<int>,std::set<int> > > missed;
    for (it = chords.begin(); it != chords.end(); ++it) {
        for (jt = chords.begin(); jt != chords.end(); ++jt) {
            if (it == jt)
                continue;
            std::vector<Transition> cl = elementary_classes(*it, *jt, k, p, z, aug);
            if (simp)
                simplify_enharmonic_classes(cl);
#if 0
            std::set<ipair> pmn = it->Pmn_relations(*jt);
            std::set<ipair>::const_iterator pt;
            int D = it->vl_efficiency_metric(*jt);
            bool has_efficient = false;
            for (std::vector<Transition>::const_iterator tt = cl.begin(); tt != cl.end(); ++tt) {
                ipair ip = tt->mn_type();
                if ((pt = pmn.find(ip)) != pmn.end()) {
                    if (!has_efficient && pt->first + 2 * pt->second <= D)
                        has_efficient = true;
                    pmn.erase(pt);
                }
            }
            if (!has_efficient) {
                for (pt = pmn.begin(); pt != pmn.end(); ++pt) {
                    if (pt->first + 2 * pt->second > D)
                        continue;
                    std::vector<int> lst(3);
                    lst[0] = pt->first;
                    lst[1] = pt->second;
                    lst[2] = Tone::modb(jt->root() - it->root(), 12);
                    std::set<int> ts;
                    ts.insert(it->type());
                    ts.insert(jt->type());
                    missed.insert(std::make_pair(lst, ts));
                }
            }
#endif
            for (kt = cl.begin(); kt != cl.end(); ++kt) {
                bool found = false;
                for (st = cls.begin(); st != cls.end(); ++st) {
                    if (kt->is_structurally_equal(*st) || (p == NO_PREPARATION && kt->is_structurally_equal(st->retrograde()))) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    if (kt->is_closer_than(*st, 0)) {
                        cls.erase(st);
                        cls.push_back(*kt);
                    }
                } else cls.push_back(*kt);
            }
        }
    }
#if 0
    for (std::set<std::pair<std::vector<int>,std::set<int> > >::const_iterator it = missed.begin(); it != missed.end(); ++it) {
        std::cerr << "Missed Pmn-relation: (" << it->first[0] << "," << it->first[1] << "), " << it->first[2]
                  << ", " << *(it->second.begin()) << ", " << it->second.size()
                  << std::endl;
    }
#endif
    std::sort(cls.begin(), cls.end());
    return cls;
}

std::set<std::vector<Transition> > Transition::enharmonic_classes(const std::vector<Transition> &st) {
    std::map<Transition,std::vector<Transition> > m;
    std::map<Transition,std::vector<Transition> >::iterator mt;
    for (std::vector<Transition>::const_iterator it = st.begin(); it != st.end(); ++it) {
        for (mt = m.begin(); mt != m.end(); ++mt) {
            if (it->is_enharmonically_equal(mt->first))
                break;
        }
        if (mt != m.end())
            mt->second.push_back(*it);
        else
            m[*it].push_back(*it);
    }
    std::set<std::vector<Transition> > ret;
    for (mt = m.begin(); mt != m.end(); ++mt) {
        ret.insert(mt->second);
    }
    return ret;
}

void Transition::simplify_enharmonic_class(std::vector<Transition> &st) {
    if (st.empty())
        return;
    std::map<int,std::vector<Transition> > m, n;
    for (std::vector<Transition>::const_iterator it = st.begin(); it != st.end(); ++it) {
        m[it->augmented_count()].push_back(*it);
    }
    st = m[0];
    if (st.empty())
        st = m[1];
    if (st.empty())
        st = m[2];
    for (std::vector<Transition>::const_iterator it = st.begin(); it != st.end(); ++it) {
        n[it->degree()].push_back(*it);
    }
    for (std::map<int,std::vector<Transition> >::const_iterator it = n.begin(); it != n.end(); ++it) {
        if (!it->second.empty()) {
            st = it->second;
            break;
        }
    }
    assert(!st.empty());
}

void Transition::simplify_enharmonic_classes(std::vector<Transition> &cl) {
    std::set<std::vector<Transition> > ecl = Transition::enharmonic_classes(cl);
    cl.clear();
    for (std::set<std::vector<Transition> >::const_iterator et = ecl.begin(); et != ecl.end(); ++et) {
        std::vector<Transition> tv = *et;
        simplify_enharmonic_class(tv);
        cl.insert(cl.end(), tv.begin(), tv.end());
    }
}

std::ostream& operator <<(std::ostream &os, const Transition &t) {
    os << t.to_string();
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::vector<Transition> &tv) {
    for (std::vector<Transition>::const_iterator it = tv.begin(); it != tv.end(); ++it) {
        os << it->to_string() << std::endl;
    }
    return os;
}
