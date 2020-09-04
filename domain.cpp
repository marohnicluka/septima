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
 * Septima is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Septima.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "domain.h"
#include <string.h>

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

bool Domain::str_to_int(const char *str, int &res) {
    if (strlen(str) == 1 && str[0] == 0) {
        res = 0;
        return true;
    }
    res = atoi(str);
    return res != 0;
}

Domain Domain::parse(char *spec) {
    Domain ret;
    const char *delim = ",;";
    char *pch = strtok(spec, delim);
    char num[1024];
    int m, n;
    while (pch != NULL) {
        if (strlen(pch) > 0) {
            strcpy(num, pch);
            size_t i = 0, len = strlen(num);
            for (; i < len; ++i) {
                if (num[i] == ':') {
                    num[i] == '\0';
                    break;
                }
            }
            if (i + 1 < len && str_to_int(num, m) && str_to_int(num+i+1, n))
                ret.insert_range(m, n);
            else if (i == len && str_to_int(num, n))
                ret.insert(n);
        }
        pch = strtok(NULL, delim);
    }
    return ret;
}

std::ostream& operator <<(std::ostream &os, const Domain &d) {
    int n = d.size(), i = 0;
    os << "{";
    for (Domain::const_iterator it = d.begin(); it != d.end(); ++it) {
        os << *it;
        if (++i < n)
            os << ",";
    }
    os << "}";
    return os;
}
