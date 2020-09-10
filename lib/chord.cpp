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
 * Septima is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Septima.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "chord.h"
#include "tone.h"
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <cstring>

const char* Chord::symbols[] = {
    "d7", "hdim7", "m7", "maj7", "dim7", "Ger6+", "TC"
};

const char* Chord::note_names[] = {
    "c", "cis", "d", "es", "e", "f", "fis", "g", "as", "a", "bes", "b"
};

Chord::Chord() {
    _root = 0;
    _type = -1;
}

Chord::Chord(int r, int t) {
    _root = Tone::modb(r, 12);
    _type = t;
}

Chord::Chord(const char *symbol) {
    int pos = 0, t = 0, len = strlen(symbol);
    char *s = new char[len+1];
    strcpy(s, symbol);
    for (; pos < len && s[pos] != ':'; ++pos);
    if (pos == len) {
        _root = _type = -1;
    } else {
        s[pos] = '\0';
        _root = Tone::modb(atoi(symbol), 12);
        if (_root == 0 && (strlen(s) == 0 || strlen(s) > 1 || s[0] != '0'))
            _root = -1;
        else {
            for (; t < 5 && strcmp(s+pos+1, symbols[t]); ++t);
            _type = t;
        }
    }
    delete[] s;
    if (is_valid() && _type == DIMINISHED_SEVENTH)
        _root = Tone::modb(_root, 3);
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

bool Chord::is_valid() const {
    return 0 <= _type && _type < 5 && 0 <= _root && _root < 12;
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

std::string Chord::to_string() const {
    char res[8];
    sprintf(res, "%d:%s", _root, symbols[_type]);
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
    case DOMINANT_SEVENTH:
        ss << "^7";
        break;
    case HALF_DIMINISHED_SEVENTH:
        ss << "^\\text{\\o}";
        break;
    case MINOR_SEVENTH:
        ss << "\\mathrm{m}^7";
        break;
    case MAJOR_SEVENTH:
        ss << "^\\triangle ";
        break;
    case DIMINISHED_SEVENTH:
        ss << "^{\\mathrm{o}7}";
        break;
    default:
        assert(false);
    }
    return ss.str();
}

std::string Chord::to_lily(int duration) const {
    std::stringstream ss;
    ss << note_names[root()];
    if (duration > 0)
        ss << duration;
    ss << ":";
    switch (type()) {
    case DOMINANT_SEVENTH:
        ss << "7";
        break;
    case HALF_DIMINISHED_SEVENTH:
        ss << "m7.5-";
        break;
    case MINOR_SEVENTH:
        ss << "m7";
        break;
    case MAJOR_SEVENTH:
        ss << "maj7";
        break;
    case DIMINISHED_SEVENTH:
        ss << "dim7";
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
    if (t == DOMINANT_SEVENTH)
        t = HALF_DIMINISHED_SEVENTH;
    else if (t == DOMINANT_SEVENTH)
        t = HALF_DIMINISHED_SEVENTH;
    else if (t == GERMAN_SIXTH)
        t = TRISTAN_CHORD;
    else if (t == TRISTAN_CHORD)
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

std::vector<Chord> Chord::make_sequence_from_symbols(const char* symbols[], int len) {
    std::vector<Chord> seq;
    for (int i = 0; i < len; ++i) {
        seq.push_back(Chord(symbols[i]));
    }
    return seq;
}

std::vector<Chord> Chord::dominant_seventh_chords() {
    std::vector<Chord> lst;
    for (int i = 0; i < 12; ++i) {
        lst.push_back(Chord(i, DOMINANT_SEVENTH));
    }
    return lst;
}

std::vector<Chord> Chord::half_diminished_seventh_chords() {
    std::vector<Chord> lst;
    for (int i = 0; i < 12; ++i) {
        lst.push_back(Chord(i, HALF_DIMINISHED_SEVENTH));
    }
    return lst;
}

std::vector<Chord> Chord::minor_seventh_chords() {
    std::vector<Chord> lst;
    for (int i = 0; i < 12; ++i) {
        lst.push_back(Chord(i, MINOR_SEVENTH));
    }
    return lst;
}

std::vector<Chord> Chord::major_seventh_chords() {
    std::vector<Chord> lst;
    for (int i = 0; i < 12; ++i) {
        lst.push_back(Chord(i, MAJOR_SEVENTH));
    }
    return lst;
}

std::vector<Chord> Chord::diminished_seventh_chords() {
    std::vector<Chord> lst;
    for (int i = 0; i < 3; ++i) {
        lst.push_back(Chord(i, DIMINISHED_SEVENTH));
    }
    return lst;
}

std::vector<Chord> Chord::all_seventh_chords() {
    std::vector<Chord> lst;
    std::vector<Chord> dom = dominant_seventh_chords();
    std::vector<Chord> hdim = half_diminished_seventh_chords();
    std::vector<Chord> min = minor_seventh_chords();
    std::vector<Chord> maj = major_seventh_chords();
    std::vector<Chord> dim = diminished_seventh_chords();
    lst.insert(lst.end(), dom.begin(), dom.end());
    lst.insert(lst.end(), hdim.begin(), hdim.end());
    lst.insert(lst.end(), min.begin(), min.end());
    lst.insert(lst.end(), maj.begin(), maj.end());
    lst.insert(lst.end(), dim.begin(), dim.end());
    return lst;
}

std::ostream& operator <<(std::ostream &os, const Chord &c) {
    os << c.to_string();
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::vector<Chord> &cv) {
    int n = cv.size();
    os << "[";
    for (int i = 0; i < n; ++i) {
        os << cv.at(i).to_string();
        if (i != n - 1)
            os << ",";
    }
    os << "]";
    return os;
}

std::ostream& operator <<(std::ostream &os, const ipair &ip) {
    os << "(" << ip.first << "," << ip.second << ")";
    return os;
}
