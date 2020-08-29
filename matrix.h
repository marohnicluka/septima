/* matrix.h
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

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <gsl/gsl_matrix_double.h>
#include <gsl/gsl_linalg.h>

class Matrix {

    int _size;
    std::vector<double> _elm;
    void assign(const Matrix &other);

public:
    Matrix(int n);
    Matrix(const Matrix &other);
    ~Matrix() { }
    Matrix &operator =(const Matrix &other);
    bool operator ==(const Matrix &other) const;
    bool operator !=(const Matrix &other) const;

    int size() const;
    /* returns the order of this (square) matrix */

    gsl_matrix *to_gsl_matrix() const;
    /* convert this matrix to gsl_matrix */

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
    /* returns the matrix exponential using GSL */

    Matrix inverse() const;
    /* returns the inverse of this matrix using GSL */

    std::vector<std::pair<double,double> > eigenvalues() const;
    /* returns the eigenvalues of this matrix as a vector of pairs e=(x,y) where x=Re(e) and y=Im(e), using GSL */

    void print() const;
    /* outputs the matrix on stdout */

    static Matrix identity(int n);
    /* returns the identity matrix of order n */
};

#endif // MATRIX_H
