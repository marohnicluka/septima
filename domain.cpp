/* domain.cpp
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
