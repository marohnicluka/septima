/* domain.cpp
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

#include "domain.h"

void Domain::insert_range(int lb, int ub) {
    for (int k = lb; k <= ub; ++k) {
        insert(Tone(k));
    }
}

int Domain::lbound() const {
    return begin()->lof_position();
}

int Domain::ubound() const {
    return rbegin()->lof_position();
}

int Domain::diameter() const {
    return ubound() - lbound();
}

bool Domain::contains(const std::set<Tone> &s) const {
    for (std::set<Tone>::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (find(*it) == end())
            return false;
    }
    return true;
}

Domain Domain::usual() {
    Domain dom;
    dom.insert_range(-15, 15);
    return dom;
}
