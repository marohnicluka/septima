/* matrix.h
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

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <gsl/gsl_matrix_double.h>
#include <gsl/gsl_linalg.h>

class Matrix {

    int _size;
    std::vector<double> _elm;
    void assign(const Matrix &other);
    gsl_matrix *to_gsl_matrix() const;

public:
    Matrix(int n);
    Matrix(const Matrix &other);
    ~Matrix() { }
    Matrix &operator =(const Matrix &other);
    bool operator ==(const Matrix &other) const;
    bool operator !=(const Matrix &other) const;

    int size() const;
    /* returns the order of this (square) matrix */

    void set_element(int i, int j, double e);
    /* sets the element at (i,j) -- note that indices are 1-based */

    double element(int i, int j) const;
    /* returns the element at (i,j) */

    const std::vector<double> &elements() const;
    /* returns the non-mutable list of elements */

    void scale(double s);
    /* scales the elements of the matrix by the factor s */

    void add(const Matrix &other);
    /* adds other to this matrix */

    void mul(const Matrix &other, int row = 0, int col = 0);
    /* multiplies this matrix by other */

    Matrix exponential() const;
    /* returns the matrix exponential */

    Matrix inverse() const;
    /* returns the inverse of this matrix */

    std::vector<std::pair<double,double> > eigenvalues() const;
    /* returns the eigenvalues of this matrix as a vector of pairs e=(x,y) where x=Re(e) and y=Im(e) */

    void print() const;
    /* outputs the matrix on stdout */

    static Matrix identity(int n);
    /* returns the identity matrix of order n */
};

#endif // MATRIX_H
