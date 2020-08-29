/* tone.cpp
 *
 * Copyright (c) 2020  Luka Marohnić
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

#include "tone.h"
#include <math.h>
#include <assert.h>

Tone::Tone() {
    _lof = 0;
}

Tone::Tone(int lof) {
    _lof = lof;
}

Tone::Tone(const Tone &other) {
    _lof = other.lof_position();
}

Tone& Tone::operator =(const Tone &other) {
    _lof = other.lof_position();
    return *this;
}

bool Tone::operator ==(const Tone &other) const {
    return _lof == other.lof_position();
}

bool Tone::operator !=(const Tone &other) const {
    return !(*this == other);
}

bool Tone::operator <(const Tone &other) const {
    return this->lof_position() < other.lof_position();
}

int Tone::lof_position() const {
    return _lof;
}

int Tone::note_name() const {
    return modb(4 * _lof + 1, 7);
}

int Tone::pitch_class() const {
    return modb(7 * _lof + 2, 12);
}

int Tone::accidental() const {
    int lof = _lof, acc = 0, dir = (_lof > 0 ? -1 : 1);
    while (lof > 3 || lof < -3) {
        lof += dir * 7;
        acc -= dir;
    }
    return acc;
}

void Tone::transpose(int steps) {
    _lof += steps;
}

std::string Tone::to_string() const {
    const char* names = "CDEFGAB";
    char res[64];
    res[0] = names[note_name()];
    int acc = accidental(), i = 0;
    for (; i < abs(acc) && i < 62; ++i) {
        res[i+1] = (acc < 0 ? 'b' : '#');
    }
    res[i+1] = '\0';
    return std::string(res);
}

std::string Tone::to_lily() const {
    const char* names = "cdefgab";
    char res[64];
    int acc = accidental();
    res[0] = names[note_name()];
    for (int i = 0; i < abs(acc); ++i) {
        res[2*(i+1)] = 's';
        res[2*i+1] = (acc < 0 ? 'e' : 'i');
    }
    res[2*abs(acc)+1] = '\0';
    return std::string(res);
}

ipair Tone::interval(const Tone &other) const {
    int generic_size = modb(other.note_name() - this->note_name(), 7);
    int specific_size = modb(other.pitch_class() - this->pitch_class(), 12);
    return std::make_pair(generic_size, specific_size);
}

Tone Tone::structural_inversion() const {
    Tone ret(-this->lof_position());
    return ret;
}

ipair Tone::interval_abs(const Tone &a, const Tone &b) {
    ipair intrv = a.interval(b);
    if (intrv.second <= 6)
        return intrv;
    return b.interval(a);
}

int Tone::lof_distance(const Tone &a, const Tone &b) {
    return abs(a.lof_position() - b.lof_position());
}

int Tone::modb(int k, int b) {
    int n = k;
    while (n < 0) n += b;
    return n % b;
}

int Tone::modd(int k, int b) {
    int n = modb(k, b);
    if (n > b / 2)
        return abs(n - b);
    return n;
}

int Tone::pitch_class_to_lof(int pc) {
    int k = 0;
    while (true) {
        if (Tone(k).pitch_class() == pc)
            return k;
        if (Tone(-k).pitch_class() == pc)
            return -k;
        ++k;
    }
    assert(false); // unreachable
}

std::ostream& operator <<(std::ostream &os, const Tone &t) {
    os << t.to_string();
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::vector<Tone> &tv) {
    int n = tv.size(), i = 0;
    for (std::vector<Tone>::const_iterator it = tv.begin(); it != tv.end(); ++it) {
        ++i;
        os << it->to_string();
        if (i != n)
            os << ",";
    }
    return os;
}

std::ostream& operator <<(std::ostream &os, const std::set<Tone> &ts) {
    int n = ts.size(), i = 0;
    for (std::set<Tone>::const_iterator it = ts.begin(); it != ts.end(); ++it) {
        ++i;
        os << it->to_string();
        if (i != n)
            os << ",";
    }
    return os;
}
