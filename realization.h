/* realization.h
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

#ifndef REALIZATION_H
#define REALIZATION_H

#include "tone.h"
#include "chord.h"
#include <vector>

class Realization {
    Tone _tones[4];
    Chord _chord;
public:
    Realization() { }
    Realization(const Chord &c); // construct a realization for the chord c
    Realization(const Realization &other);
    ~Realization() { }
    Realization& operator =(const Realization &other);
    bool operator ==(const Realization &other) const;

    bool is_equivalent(const Realization &other) const;
    /* is this realization enharmonically equivalent to the other? */

    bool is_valid() const;
    /* is the realization valid? */

    bool is_augmented(bool tristan = false) const;
    /* is this an augmented chord (German sixth or Tristan)? */

    int root_voice() const;
    /* return the voice containing the generic root of the chord */

    int seventh_voice() const;
    /* return the voice containing the acoustic seventh or -1 if there is none */

    int chord_type() const;
    /* return the type of the realized chord */

    int acc_weight(int key = 0) const;
    /* return the sum of abs(_acc) accross tones */

    bool requires_preparation() const;
    /* does the seventh requires preparation? */

    void arrange(const std::vector<int> &perm);
    /* rearrange tones */

    const Tone& tone(int i) const;
    Tone& tone(int i);
    std::string to_string() const;
    static const int sym4[][4];

    static std::vector<Realization> generate(const Chord &c);
    /* return the list of all tonal realizations of the chord c */
};

#endif // REALIZATION_H
