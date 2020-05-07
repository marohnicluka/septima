/* realization.cpp
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

#include "realization.h"
#include <assert.h>
#include <set>

Realization::Realization(const Chord &c) {
    _chord = c;
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
    bool yes = _chord == other._chord;
    for (int i = 0; yes && i < 4; ++i) {
        yes = _tones[i] == other.tone(i);
    }
    return yes;
}

bool Realization::is_equivalent(const Realization &other) const {
    std::set<int> P1, P2;
    for (int i = 0; i < 4; ++i) {
        P1.insert(_tones[i].pitch());
        P2.insert(other.tone(i).pitch());
    }
    return P1 == P2;
}

bool Realization::is_valid() const {
    std::set<int> p, q;
    for (int i = 0; i < 4; ++i) {
        if (!_tones[i].is_valid()) return false;
        p.insert(_tones[i].pitch() % 12);
    }
    q.insert(_chord.root());
    q.insert(_chord.third());
    q.insert(_chord.fifth());
    q.insert(_chord.seventh());
    return p == q;
}

bool Realization::is_augmented(bool tristan) const {
    int t = chord_type();
    return t == TRISTAN || (!tristan && t == GERMAN_SIXTH);
}

const Tone &Realization::tone(int i) const {
    assert(i >= 0 && i < 4);
    return _tones[i];
}

Tone &Realization::tone(int i) {
    assert(i >= 0 && i < 4);
    return _tones[i];
}

int Realization::root_voice() const {
    int cnt = 0, ret = -1;
    std::pair<int,int> ip;
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

int Realization::seventh_voice() const {
    int p1, p2, dp;
    for (int i = 0; i < 4; ++i) {
        p1 = _tones[i].pitch();
        for (int j = 0; j < 4; ++j) {
            p2 = _tones[j].pitch();
            dp = Tone::mod12(p2 - p1);
            if (dp == 1 || dp == 2)
                return i;
        }
    }
    return -1;
}

int Realization::chord_type() const {
    std::pair<int,int> ip;
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

int Realization::acc_weight(int key) const {
    int ret = 0;
    for (int i = 0; i < 4; ++i) {
        ret += abs(tone(i).accidental(key));
    }
    return ret;
}

bool Realization::requires_preparation() const {
    int t = chord_type();
    return t != DOMINANT && t != DIMINISHED && t != GERMAN_SIXTH;
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

std::string Realization::to_string() const {
    std::string ret = "";
    for (int i = 0; i < 4; ++i) {
        ret += _tones[i].to_string();
        if (i < 3) ret += "-";
    }
    return ret;
}

const int Realization::sym4[][4] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
    {0, 3, 1, 2}, {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2},
    {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
    {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0},
    {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 0, 2, 1},
    {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
};

std::vector<Realization> Realization::generate(const Chord &c) {
    std::vector<Realization> ret;
    int p[4], t[4], a, w, d;
    bool ok;
    p[0] = c.root ();
    p[1] = c.third ();
    p[2] = c.fifth ();
    p[3] = c.seventh ();
    for (t[0] = 0; t[0] < 4; t[0]++) {
        for (t[1] = t[0] + 1; t[1] < 5; t[1]++) {
            for (t[2] = t[1] + 1; t[2] < 6; t[2]++) {
                for (t[3] = t[2] + 1; t[3] < 7; t[3]++) {
                    for (int i = 0; i < 24; i++) {
                        ok = true;
                        Realization r(c);
                        for (int j = 0; ok && j < 4; j++) {
                            r.tone(j).set_degree(t[j]);
                            a = Tone::mod12(p[sym4[i][j]] - r.tone(j).pitch_base());
                            if (a > 6) a -= 12;
                            r.tone(j).set_accidental(a);
                            ok = r.tone(j).is_valid();
                        }
                        for (int j = 0; ok && j < 3; j++) {
                            for (int k = j + 1; ok && k < 4; k++) {
                                std::pair<int,int> ip = r.tone(j).interval(r.tone(k));
                                w = ip.first; d = ip.second;
                                if ((w == 3 && d != 5 && d != 6) || (w == 4 && d != 7 && d != 6))
                                    ok = false;
                            }
                        }
                        if (ok) ret.push_back(r);
                    }
                }
            }
        }
    }
    return ret;
}
