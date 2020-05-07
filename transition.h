/* transition.h
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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "realization.h"

class Transition {
    Realization _first;
    Realization _second;
public:
    Transition() { }
    Transition(const Realization &a, const Realization &b); // construct transition from a pair of realizations
    Transition(const Transition &other);
    Transition& operator =(const Transition &other);
    bool operator ==(const Transition &other) const;

    void set_first(const Realization &r);
    /* set the first chord */

    void set_second(const Realization &r);
    /* set the second chord */

    const Realization &first() const;
    /* get the first chord */

    const Realization &second() const;
    /* get the second chord */

    bool is_link(const Realization &pred, int &mc, std::vector<int> &f) const;
    /* Can this transition be appended to pred? (mc is the number of mandatory cues, f the voice leading) */

    int chromatic_count() const;
    /* return the number of chromatic movements in this transition */

    int vls(bool dir = false) const;
    /* return the voice-leading size of this transition */

    int vl_type() const;
    /* return the generic size of the interval from the first root to the second root */

    int vld() const;
    /* return voice-leading direction (-1 for down, 1 for up, 0 for none) for parsimonious transition */

    std::string to_string() const;
    /* return the string representation */

    static std::vector<Transition> generate_parsimonious(const Chord &c1, const Chord &c2);
    /* return the list of all parsimonious transitions from c1 to c2 */
};

#endif // TRANSITION_H
