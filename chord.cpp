/* chord.cpp
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

#include "chord.h"
#include "tone.h"

Chord::Chord() {
    _root = 0;
    _type = -1;
}

Chord::Chord(int r, int t) {
    _root = Tone::mod12(r);
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

int Chord::root() const {
    return _root;
}

int Chord::type() const {
    return _type;
}

void Chord::set_root(int r) {
    _root = Tone::mod12(r);
}

void Chord::set_type(int t) {
    _type = t % 5;
}

const int Chord::structure[][3] = {
    {4, 7, 10}, {3, 6, 10}, {3, 7, 10}, {4, 7, 11}, {3, 6, 9}
};

int Chord::third() const {
    return Tone::mod12(_root + structure[_type % 5][0]);
}

int Chord::fifth() const {
    return Tone::mod12(_root + structure[_type % 5][1]);
}

int Chord::seventh() const {
    return Tone::mod12(_root + structure[_type % 5][2]);
}

const char* Chord::type_string[] = {
    "d7", "hdim7", "m7", "maj7", "dim7", "Ger6+", "Tristan"
};

std::string Chord::to_string() const {
    char res[8];
    sprintf(res, "%d:%s", _root, type_string[_type]);
    return std::string(res);
}
