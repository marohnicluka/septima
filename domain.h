/* domain.h
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
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
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
