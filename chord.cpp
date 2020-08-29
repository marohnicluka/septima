/* chord.cpp
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

#include "chord.h"
#include "tone.h"
#include <algorithm>
#include <sstream>
#include <assert.h>

Chord::Chord() {
    _root = 0;
    _type = -1;
}

Chord::Chord(int r, int t) {
    _root = Tone::modb(r, 12);
    _type = t % 5;
}

Chord::Chord(const Chord &other) {
    _root = other.root();
    _type = other.type();
}

Chord& Chord::operator =(const Chord &other) {
    _root = other.root();
    _type = other.type();
    return *this;
}

bool Chord::operator ==(const Chord &other) const {
    return _root == other.root() && _type == other.type();
}

bool Chord::operator !=(const Chord &other) const {
    return !(*this == other);
}

int Chord::root() const {
    return _root;
}

int Chord::type() const {
    return _type;
}

void Chord::set_root(int r) {
    _root = Tone::modb(r, 12);
}

void Chord::set_type(int t) {
    _type = t % 5;
}

const int Chord::structure[][3] = {
    {4, 7, 10}, {3, 6, 10}, {3, 7, 10}, {4, 7, 11}, {3, 6, 9}
};

int Chord::third() const {
    return Tone::modb(_root + structure[_type % 5][0], 12);
}

int Chord::fifth() const {
    return Tone::modb(_root + structure[_type % 5][1], 12);
}

int Chord::seventh() const {
    return Tone::modb(_root + structure[_type % 5][2], 12);
}

const char* Chord::type_string[] = {
    "d7", "hdim7", "m7", "maj7", "dim7", "Ger6+", "TC"
};

std::string Chord::to_string() const {
    char res[8];
    sprintf(res, "%d:%s", _root, type_string[_type]);
    return std::string(res);
}

std::string Chord::to_tex() const {
    Tone rt = Tone(Tone::pitch_class_to_lof(_root));
    int name = rt.note_name(), acc = rt.accidental(), abs_acc = abs(acc);
    const char *note_names = "CDEFGAB";
    std::stringstream ss;
    ss << "\\mathrm{" << note_names[name] << "}";
    for (int i = 0; i < abs_acc; ++i) {
        ss << (acc > 0 ? "\\sharp" : "\\flat");
    }
    switch (_type) {
    case DOMINANT:
        ss << "^7";
        break;
    case HALF_DIMINISHED:
        ss << "^\\text{\\o}";
        break;
    case MINOR:
        ss << "\\mathrm{m}^7";
        break;
    case MAJOR:
        ss << "^\\triangle ";
        break;
    case DIMINISHED:
        ss << "^{\\mathrm{o}7}";
        break;
    default:
        assert(false);
    }
    return ss.str();
}

std::set<int> Chord::pitch_class_set() const {
    std::set<int> Pc;
    Pc.insert(root());
    Pc.insert(third());
    Pc.insert(fifth());
    Pc.insert(seventh());
    return Pc;
}

Chord Chord::structural_inversion() const {
    int r = Tone::modb(4 - this->root(), 12);
    int t = this->type();
    if (t == DOMINANT)
        t = HALF_DIMINISHED;
    else if (t == DOMINANT)
        t = HALF_DIMINISHED;
    else if (t == GERMAN_SIXTH)
        t = TRISTAN;
    else if (t == TRISTAN)
        t = GERMAN_SIXTH;
    return Chord(r, t);
}

void Chord::set_differences(std::set<int> &pc1, std::set<int> &pc2, std::vector<int> &X, std::vector<int> &Y) {
    X.clear();
    Y.clear();
    std::set<int> intr;
    for (std::set<int>::const_iterator it = pc1.begin(); it != pc1.end(); ++it) {
        if (pc2.find(*it) != pc2.end())
            intr.insert(*it);
    }
    for (std::set<int>::const_iterator it = intr.begin(); it != intr.end(); ++it) {
        pc1.erase(*it);
        pc2.erase(*it);
    }
    for (std::set<int>::const_iterator it = pc1.begin(); it != pc1.end(); ++it) {
        X.push_back(*it);
    }
    for (std::set<int>::const_iterator it = pc2.begin(); it != pc2.end(); ++it) {
        Y.push_back(*it);
    }
}

std::set<ipair> Chord::Pmn_relations(const Chord &other) const {
    std::set<int> pc1 = this->pitch_class_set(), pc2 = other.pitch_class_set();
    std::vector<int> X, Y, p;
    set_differences(pc1, pc2, X, Y);
    int n = X.size();
    for (int k = 0; k < n; ++k) {
        p.push_back(k);
    }
    bool yes;
    int c1, c2, d;
    std::set<ipair> res;
    while (true) {
        yes = true;
        c1 = c2 = 0;
        for (int k = 0; yes && k < n; ++k) {
            d = Tone::modd(X[k] - Y[p[k]], 12);
            if (d == 1)
                ++c1;
            else if (d == 2)
                ++c2;
            yes = (d <= 2);
        }
        if (yes)
            res.insert(std::make_pair(c1, c2));
        if (!std::next_permutation(p.begin(), p.end()))
            break;
    }
    return res;
}

int Chord::vl_efficiency_metric(const Chord &other) const {
    std::set<int> pc1 = this->pitch_class_set(), pc2 = other.pitch_class_set();
    std::vector<int> X, Y, p;
    set_differences(pc1, pc2, X, Y);
    int n = X.size();
    for (int k = 0; k < n; ++k) {
        p.push_back(k);
    }
    int w, minw = RAND_MAX;
    while (true) {
        w = 0;
        for (int k = 0; k < n; ++k) {
            w += Tone::modd(X[k] - Y[p[k]], 12);
        }
        if (w < minw)
            minw = w;
        if (!std::next_permutation(p.begin(), p.end()))
            break;
    }
    return minw;
}

std::ostream& operator <<(std::ostream &os, const Chord &c) {
    os << c.to_string();
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::vector<Chord> &cv) {
    int n = cv.size();
    for (int i = 0; i < n; ++i) {
        os << cv.at(i).to_string();
        if (i != n - 1)
            os << ",";
    }
    return os;
}
