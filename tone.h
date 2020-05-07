/* tone.h
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

#ifndef TONE_H
#define TONE_H

#include <string>
#include <iostream>

class Tone {
    int _deg;
    int _acc;

public:
    Tone();
    Tone(int p, int a); // construct the tone with degree p and accidental a
    Tone(const Tone &other);
    ~Tone() { }
    Tone& operator =(const Tone &other);
    bool operator ==(const Tone &other) const;

    void set_degree(int p);
    /* set the generic position in staff */

    void set_accidental(int a);
    /* shift the generic position for oct octaves */

    bool is_valid() const;
    /* does this tone exist in some tonality with at most 7 sharps/flats? */

    int degree() const;
    /* return the degree in staff (0 - C, 1 - D, 2 - E, 3 - F, 4 - G, 5 - A, 6 - B) */

    int accidental(int key = 0) const;
    /* return the accidental */

    int pitch() const;
    /* return the MIDI pitch */

    int pitch_base() const;
    /* return the base MIDI pitch (between 0 and 11, inclusive) */

    std::pair<int,int> interval(const Tone &other) const;
    /* compute the interval between this tone and the other */

    static std::pair<int,int> interval_abs(const Tone &a, const Tone &b);
    /* compute the absolute interval between a and b */

    static bool is_diatonic(const Tone &a, const Tone &b);
    /* is the melodic interval between a and b diatonic? */

    static bool is_chromatic(const Tone &a, const Tone &b);
    /* is the melodic interval between a and b chromatic? */

    static bool is_smooth(const Tone &a, const Tone &b);
    /* is the melodic interval between a and b diatonic/chromatic/unison? */

    std::string to_string() const;
    /* return the string representation of the tone (use lily=true to get Lilypond code) */

    /* helper routines */
    static int mod12(int k);
    static int mod7(int k);

    /* availability arrays of double sharps/flats */
    static int double_sharps[];
    static int double_flats[];
};

#endif // TONE_H
