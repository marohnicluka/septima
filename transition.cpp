/* transition.cpp
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

#include "transition.h"
#include <assert.h>

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

bool Transition::operator ==(const Transition &other) const {
    return _first == other.first() && _second == other.second();
}

int Transition::chromatic_count() const {
    int c = 0;
    for (int i = 0; i < 4; ++i) {
        if (_first.tone(i).pitch() != _second.tone(i).pitch() &&
                _first.tone(i).degree() == _second.tone(i).degree())
            ++c;
    }
    return c;
}

bool Transition::is_link(const Realization &pred, int &mc, std::vector<int> &f) const {
    if (!pred.is_equivalent(_first))
        return false;
    f = std::vector<int>(4, -1);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (pred.tone(i).pitch() == _first.tone(j).pitch()) {
                f[i] = j;
                break;
            }
        }
    }
    mc = 0;
    bool r = true;
    for (int i = 0; i < 4; ++i) {
        if (pred.tone(i).degree() != _first.tone(f[i]).degree())
            ++mc;
        if (r && !Tone::is_smooth(pred.tone(i), _second.tone(f[i])))
            r = false;
    }
    if (r)
        mc = 0;
    return true;
}

int Transition::vls(bool dir) const {
    int e = 0;
    for (int i = 0; i < 4; ++i) {
        int d = _first.tone(i).interval(_second.tone(i)).second;
        if (d > 6)
            d -= 12;
        e += dir ? d : abs(d);
    }
    return e;
}

int Transition::vl_type() const {
    return _first.tone(_first.root_voice()).interval(_second.tone(_second.root_voice())).first;
}

int Transition::vld() const {
    int dr = vl_type();
    if (dr == 0)
        return 0;
    return 1 - 2 * (dr % 2);
}

std::string Transition::to_string() const {
    std::string s1 = _first.to_string(), s2 = _second.to_string();
    return s1 + " -> " + s2;
}

std::vector<Transition> Transition::generate_parsimonious(const Chord &c1, const Chord &c2) {
    std::vector<Realization> br1 = Realization::generate(c1);
    std::vector<Realization> br2 = Realization::generate(c2);
    std::vector<Transition> ret;
    for (std::vector<Realization>::const_iterator it = br1.begin(); it != br1.end(); ++it) {
        for (std::vector<Realization>::const_iterator jt = br2.begin(); jt != br2.end(); ++jt) {
            int dr = it->tone(it->root_voice()).interval(jt->tone(jt->root_voice())).first, cnd;
            std::vector<int> f(4, -1);
            for (int i = 0; i < 4; ++i) {
                cnd = -1;
                for (int j = 0; j < 4; ++j) {
                    if (it->tone(i).degree() == jt->tone(j).degree()) {
                        cnd = -1;
                        f[i] = j;
                        break;
                    }
                    std::pair<int,int> intrv = it->tone(i).interval(jt->tone(j));
                    if ((dr % 2 == 0 && intrv.first == 1) || (dr % 2 != 0 && intrv.first == 6))
                        cnd = j;
                }
                if (cnd >= 0)
                    f[i] = cnd;
                assert(f[i] >= 0);
            }
            Realization r1(*it), r2(*jt);
            r2.arrange(f);
            bool ok = true;
            for (int i = 0; ok && i < 4; ++i) {
                ok = Tone::is_smooth(r1.tone(i), r2.tone(i));
            }
            if (ok) {
                ok = false;
                int i7 = r2.seventh_voice();
                if (i7 >= 0 && !r2.requires_preparation())
                    i7 = -1;
                if (i7 < 0 || r1.tone(i7) == r2.tone(i7))
                    ok = true;
            }
            if (ok) ret.push_back(Transition(r1, r2));
        }
    }
    return ret;
}
