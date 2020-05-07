/* tone.cpp
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

#include "tone.h"
#include <math.h>
#include <assert.h>

Tone::Tone() {
    _deg = 0;
    _acc = 0;
}

Tone::Tone(int p, int a) {
    _deg = mod7(p);
    _acc = a;
}

int Tone::accidental(int key) const {
    int k = abs(key), d = (key > 0 ? 3 : (d < 0 ? 6 : -1)), corr = 0;
    for (int i = 0; i < k; ++i) {
        if (degree() == d) {
            corr = (key > 0 ? 1 : -1);
            break;
        }
        if (key != 0)
            d = mod7(d + (key > 0 ? 4 : -4));
    }
    return _acc - corr;
}

void Tone::set_degree(int p) {
    _deg = mod7(p);
}

void Tone::set_accidental(int a) {
    _acc = a;
}

int Tone::degree() const {
    return _deg;
}

int Tone::pitch_base() const {
    switch (degree()) {
    case 0: return 0;
    case 1: return 2;
    case 2: return 4;
    case 3: return 5;
    case 4: return 7;
    case 5: return 9;
    case 6: return 11;
    default: break;
    }
    assert(false);
}

int Tone::pitch() const {
    return mod12(pitch_base() + _acc);
}

int Tone::double_sharps[] = { 1, 1, 0, 1, 1, 1, 0 };

int Tone::double_flats[]  = { 0, 1, 1, 0, 1, 1, 1 };

bool Tone::is_valid() const {
    return abs(_acc) < 3 && (_acc != -2 || double_flats[_deg]) && (_acc != 2 || double_sharps[_deg]);
}

std::string Tone::to_string() const {
    const char* names = "CDEFGAB";
    char res[8];
    res[0] = names[_deg];
    for (int i = 0; i < abs(_acc); ++i) {
        res[1+i] = (_acc<0?'b':'#');
    }
    res[1+abs(_acc)] = '\0';
    return std::string(res);
}

std::pair<int,int> Tone::interval(const Tone &other) const {
    int generic_size = mod7(other.degree() - this->degree());
    int specific_size = mod12(other.pitch() - this->pitch());
    return std::make_pair(generic_size, specific_size);
}

std::pair<int,int> Tone::interval_abs(const Tone &a, const Tone &b) {
    std::pair<int,int> intrv = a.interval(b);
    if (intrv.second <= 6)
        return intrv;
    return b.interval(a);
}

bool Tone::is_diatonic(const Tone &a, const Tone &b) {
    std::pair<int,int> ip = interval_abs(a, b);
    int w = ip.first, d = ip.second;
    return w == 1 && d < 3 && d > 0;
}

bool Tone::is_chromatic(const Tone &a, const Tone &b) {
    std::pair<int,int> ip = interval_abs(a, b);
    int w = ip.first, d = ip.second;
    return w == 0 && d == 1;
}

bool Tone::is_smooth(const Tone &a, const Tone &b) {
    return a == b || is_diatonic(a, b) || is_chromatic(a, b);
}

Tone::Tone(const Tone &other) {
    _deg = other.degree();
    _acc = other.accidental();
}

Tone& Tone::operator =(const Tone &other) {
    _deg = other.degree();
    _acc = other.accidental();
    return *this;
}

bool Tone::operator ==(const Tone &other) const {
    return _deg == other.degree() && _acc == other.accidental();
}

int Tone::mod7(int k) {
    int n = k;
    while (n < 0) n += 7;
    return n % 7;
}

int Tone::mod12(int k) {
    int n = k;
    while (n < 0) n += 12;
    return n % 12;
}
