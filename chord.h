/* chord.h
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

#ifndef CHORD_H
#define CHORD_H

#include <string>

/* types of seventh chord realizations */
enum ChordType {
    DOMINANT        = 0,
    HALF_DIMINISHED = 1,
    MINOR           = 2,
    MAJOR           = 3,
    DIMINISHED      = 4,
    GERMAN_SIXTH    = 5,
    TRISTAN         = 6
};

class Chord {
    int _root;
    int _type;
public:
    Chord();
    Chord(int r, int t); // construct the chord with root pitch r and type t (0-4)
    Chord(const Chord &other);
    ~Chord() { }
    Chord& operator =(const Chord &other);
    bool operator ==(const Chord &other) const;
    static const int structure[][3];
    static const char *type_string[];

    int root() const;
    /* return the root pitch class of this chord */

    int type() const;
    /* return the type of this chord (0-4) */

    void set_root(int r);
    /* set the root pitch */

    void set_type(int t);
    /* set the type (0-4) */

    int third() const;
    /* return the pitch class of the third */

    int fifth() const;
    /* return the pitch class of the fifth */

    int seventh() const;
    /* return the pitch class of the seventh */

    std::string to_string() const;
    /* return the string representation */
};

#endif // CHORD_H
