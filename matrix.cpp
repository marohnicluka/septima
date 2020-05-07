/* matrix.cpp
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

#include "matrix.h"
#include <assert.h>

Matrix::Matrix(int n) {
    _size = n;
    _elm.resize(n * n, 0);
}

Matrix::Matrix(const Matrix &other) {
    assign(other);
}

Matrix &Matrix::operator =(const Matrix &other) {
    assign(other);
    return *this;
}

bool Matrix::operator ==(const Matrix &other) const {
    if (_size != other.size())
        return false;
    return _elm == other.elements();
}

void Matrix::assign(const Matrix &other) {
    _size = other.size();
    _elm = other.elements();
}

int Matrix::size() const {
    return _size;
}

void Matrix::set_element(int i, int j, long e) {
    _elm[(i - 1) * _size + j - 1] = e;
}

long Matrix::element(int i, int j) const {
    return _elm[(i - 1) * _size + j - 1];
}

const std::vector<long> &Matrix::elements() const {
    return _elm;
}

void Matrix::scale(long s) {
    for (std::vector<long>::iterator it = _elm.begin(); it != _elm.end(); ++it) {
        if (*it != 0) *it *= s;
    }
}

void Matrix::add(const Matrix &other) {
    assert(other.size() == _size);
    for (std::vector<long>::iterator it = _elm.begin(); it != _elm.end(); ++it) {
        *it += other.elements().at(it - _elm.begin());
    }
}

Matrix Matrix::identity(int n) {
    Matrix ret(n);
    for (int i = 1; i <= n; ++i) {
        ret.set_element(i, i, 1);
    }
    return ret;
}

void Matrix::mul(const Matrix &other, int row, int col) {
    Matrix old(*this);
    long e;
    for (int i = 1; i <= _size; ++i) {
        if (row > 0 && row != i) continue;
        for (int j = 1; j <= _size; ++j) {
            if (col > 0 && col != j) continue;
            e = 0;
            for (int k = 1; k <= _size; ++k) {
                if (other.element(k, j) != 0)
                    e += old.element(i, k) * other.element(k, j);
            }
            set_element(i, j, e);
        }
    }
}
