/* realization.cpp
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

#include "realization.h"
#include <assert.h>
#include <set>
#include <algorithm>
#include <math.h>

Realization::Realization(const Chord &c) {
    _chord = c;
    _tones[0] = Tone::pitch_class_to_lof(c.root());
    _tones[1] = Tone::pitch_class_to_lof(c.third());
    _tones[2] = Tone::pitch_class_to_lof(c.fifth());
    _tones[3] = Tone::pitch_class_to_lof(c.seventh());
}

Realization::Realization(const Realization &other) {
    _chord = other._chord;
    for (int i = 0; i < 4; ++i) {
        _tones[i] = other.tone(i);
    }
}

Realization& Realization::operator =(const Realization &other) {
    _chord = other._chord;
    for (int i = 0; i < 4; ++i) {
        _tones[i] = other.tone(i);
    }
    return *this;
}

bool Realization::operator ==(const Realization &other) const {
    return this->tone_set() == other.tone_set();
}

bool Realization::operator !=(const Realization &other) const {
    return !(*this == other);
}

bool Realization::operator <(const Realization &other) const {
    return this->tone_set() < other.tone_set();
}

bool Realization::is_enharmonically_equal(const Realization &other) const {
    std::set<int> P1, P2;
    for (int i = 0; i < 4; ++i) {
        P1.insert(_tones[i].pitch_class());
        P2.insert(other.tone(i).pitch_class());
    }
    return P1 == P2;
}

bool Realization::is_augmented_sixth(bool tristan) const {
    int t = type();
    return t == TRISTAN_CHORD || (!tristan && t == GERMAN_SIXTH);
}

const Tone &Realization::tone(int i) const {
    assert(i >= 0 && i < 4);
    return _tones[i];
}

Tone &Realization::tone(int i) {
    assert(i >= 0 && i < 4);
    return _tones[i];
}

std::set<Tone> Realization::tone_set() const {
    std::set<Tone> S;
    for (int i = 0; i < 4; ++i) {
        S.insert(tone(i));
    }
    return S;
}

Realization Realization::structural_inverse() const {
    Realization ret(this->chord().structural_inversion());
    for (int i = 0; i < 4; ++i) {
        ret.tone(i) = this->tone(3 - i).structural_inversion();
    }
    return ret;
}

double Realization::lof_point_distance(int z) const {
    int sum = 0;
    std::set<Tone> X = tone_set();
    for (std::set<Tone>::const_iterator it = X.begin(); it != X.end(); ++it) {
        sum += (it->lof_position() - z) * (it->lof_position() - z);
    }
    return sqrt(sum) / 2.0;
}

int Realization::generic_root_voice() const {
    int cnt = 0, ret = -1;
    ipair ip;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            ip = _tones[i].interval(_tones[j]);
            if (ip.first == 1) {
                ret = j;
                ++cnt;
            }
        }
    }
    assert(cnt == 1);
    return ret;
}

int Realization::generic_seventh_voice() const {
    int cnt = 0, ret = -1;
    ipair ip;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            ip = _tones[i].interval(_tones[j]);
            if (ip.first == 1) {
                ret = i;
                ++cnt;
            }
        }
    }
    assert(cnt == 1);
    return ret;
}

int Realization::acoustic_seventh_voice() const {
    int p1, p2, dp;
    for (int i = 0; i < 4; ++i) {
        p1 = _tones[i].pitch_class();
        for (int j = 0; j < 4; ++j) {
            p2 = _tones[j].pitch_class();
            dp = Tone::modb(p2 - p1, 12);
            if (dp == 1 || dp == 2)
                return i;
        }
    }
    return -1;
}

int Realization::type() const {
    ipair ip;
    int cnt = 0, ret = -1;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            ip = _tones[i].interval(_tones[j]);
            if (ip.first == 1) {
                ret = _chord.type();
                assert(ret >= 0);
                if (ret < 2 && ip.second == 3) ret += 5;
                ++cnt;
            }
        }
    }
    assert(cnt == 1);
    return ret;
}

const Chord& Realization::chord() const {
    return _chord;
}

void Realization::arrange(const std::vector<int> &perm) {
    Tone tmp[4];
    for (int i = 0; i < 4; ++i) {
        tmp[i] = _tones[perm[i]];
    }
    for (int i = 0; i < 4; ++i) {
        _tones[i] = tmp[i];
    }
}

void Realization::transpose(int d) {
    _chord.set_root(Tone::modb(_chord.root() + 7 * d, 12));
    for (int i = 0; i < 4; ++i) {
        tone(i).transpose(d);
    }
}

bool Realization::check_fifths() const {
    bool yes = true;
    ipair p;
    for (int i = 0; yes && i < 4; ++i) {
        for (int j = 0; yes && j < 4; ++j) {
            if (i == j)
                continue;
            p = tone(i).interval(tone(j));
            if (p.first == 4 && p.second != 6 && p.second != 7)
                yes = false;
        }
    }
    return yes;
}

std::string Realization::to_string() const {
    std::string ret = "";
    for (int i = 0; i < 4; ++i) {
        ret += _tones[i].to_string();
        if (i < 3) ret += "-";
    }
    return ret;
}

const int Realization::lof_structure[][4] = {
    {-2, 0, 1, 4},      // dominant
    {-6, -3, -2, 0},    // half-diminished
    {-3, -2, 0, 1},     // minor
    {0, 1, 4, 5},       // major
    {-9, -6, -3, 0},    // diminished
    {0, 1, 4, 10},      // German sixth
    {0, 6, 9, 10}       // Tristan chord
};

std::vector<Realization> Realization::tonal_realizations(const Chord &c, const Domain &dom, bool aug) {
    std::vector<Realization> ret;
    int ct = c.type(), rt;
    Realization bs(c), r(c);
    for (int lof = dom.lbound(); lof <= dom.ubound(); ++lof) {
        for (int i = 0; i < 4; ++i) {
            rt = lof + lof_structure[ct][i];
            r.tone(i) = Tone(rt);
        }
        if (dom.contains(r.tone_set()) && bs.is_enharmonically_equal(r))
            ret.push_back(r);
        if (aug && (ct == 0 || ct == 1)) {
            for (int i = 0; i < 4; ++i) {
                rt = lof + lof_structure[ct+5][i];
                r.tone(i) = Tone(rt);
            }
            if (dom.contains(r.tone_set()) && bs.is_enharmonically_equal(r))
                ret.push_back(r);
        }
    }
    return ret;
}

std::set<std::vector<int> > Realization::lof_patterns(const Chord &c, int &tot, int &ton, const Domain &dom) {
    std::set<int> Pc = c.pitch_class_set(), Rp;
    int k1, k2, k3, k4;
    Realization r;
    std::vector<int> pat(4), sig(3);
    std::set<std::vector<int> > ret;
    int lb = dom.lbound(), ub = dom.ubound();
    for (k1 = lb; k1 <= ub; ++k1) {
        r.tone(0) = Tone(k1);
        for (k2 = k1 + 1; k2 <= ub; ++k2) {
            r.tone(1) = Tone(k2);
            for (k3 = k2 + 1; k3 <= ub; ++k3) {
                r.tone(2) = Tone (k3);
                for (k4 = k3 + 1; k4 <= ub; ++k4) {
                    r.tone(3) = Tone (k4);
                    if (!dom.contains(r.tone_set()))
                        continue;
                    Rp.clear();
                    for (int i = 0; i < 4; ++i) {
                        Rp.insert(r.tone(i).pitch_class());
                    }
                    if (Rp != Pc)
                        continue;
                    ++tot;
                    if (r.check_fifths()) {
                        for (int i = 0; i < 4; ++i) {
                            pat[i] = r.tone(i).lof_position();
                        }
                        std::sort(pat.begin(), pat.end());
                        for (int i = 0; i < 3; ++i) {
                            sig[i] = pat[i+1] - pat[i];
                        }
                        ret.insert(sig);
                        ++ton;
                    }
                }
            }
        }
    }
    return ret;
}

std::ostream& operator <<(std::ostream &os, const Realization &r) {
    os << r.to_string();
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::vector<Realization> &rv) {
    int n = rv.size(), i = 0;
    os << "[";
    for (std::vector<Realization>::const_iterator it = rv.begin(); it != rv.end(); ++it) {
        ++i;
        os << it->to_string();
        if (i != n)
            os << ",";
    }
    os << "]";
    return os;
}
