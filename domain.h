/* domain.h
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

#ifndef DOMAIN_H
#define DOMAIN_H

#include "tone.h"
#include <set>

class Domain : public std::set<Tone> {

public:
    void insert_range(int lb, int ub);
    /* inserts the tones from lb to ub on the line of fifths */

    int lbound() const;
    /* returns the lower bound of this domain on the line of fifths */

    int ubound() const;
    /* returns the upper bound of this domain on the line of fifths */

    int diameter() const;
    /* returns the diameter of this domain */

    bool contains(const std::set<Tone> &s) const;
    /* returns true iff s is contained in this domain */

    static Domain usual();
    /* returns the domain from -15 (Gbb) to 15 (A##) on the line of fifths */
};

#endif // DOMAIN_H
