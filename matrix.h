/* matrix.h
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

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

class Matrix {
    int _size;
    std::vector<long> _elm;
    void assign(const Matrix &other);
public:
    Matrix(int n);
    Matrix(const Matrix &other);
    ~Matrix() { }
    Matrix &operator =(const Matrix &other);
    bool operator ==(const Matrix &other) const;
    int size() const;
    void set_element(int i, int j, long e);
    long element(int i, int j) const;
    const std::vector<long> &elements() const;
    void mul(const Matrix &other, int row = 0, int col = 0);
    void add(const Matrix &other);
    void scale(long s);
    static Matrix identity(int n);
};

#endif // MATRIX_H
