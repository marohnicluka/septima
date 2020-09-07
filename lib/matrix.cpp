/* matrix.cpp
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

#include "matrix.h"
#include <assert.h>
#include <iostream>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>

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

bool Matrix::operator !=(const Matrix &other) const {
    return !(*this == other);
}

void Matrix::assign(const Matrix &other) {
    _size = other.size();
    _elm = other.elements();
}

int Matrix::size() const {
    return _size;
}

void Matrix::set_element(int i, int j, double e) {
    _elm[(i - 1) * _size + j - 1] = e;
}

double Matrix::element(int i, int j) const {
    return _elm[(i - 1) * _size + j - 1];
}

const std::vector<double> &Matrix::elements() const {
    return _elm;
}

void Matrix::scale(double s) {
    for (std::vector<double>::iterator it = _elm.begin(); it != _elm.end(); ++it) {
        if (*it != 0) *it *= s;
    }
}

void Matrix::add(const Matrix &other) {
    assert(other.size() == _size);
    for (std::vector<double>::iterator it = _elm.begin(); it != _elm.end(); ++it) {
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

Matrix Matrix::exponential() const {
    int s = _size, s2 = s * s;
    double data[s2], zdata[s2];
    for (std::vector<double>::const_iterator it = _elm.begin(); it != _elm.end(); ++it) {
        data[it-_elm.begin()] = *it;
        zdata[it-_elm.begin()] = 0.0;
    }
    gsl_matrix_const_view m = gsl_matrix_const_view_array(data, s, s);
    gsl_matrix_view em = gsl_matrix_view_array(zdata, s, s);
    gsl_linalg_exponential_ss(&m.matrix, &em.matrix, GSL_PREC_DOUBLE);
    Matrix ret(s);
    for (int i = 1; i <= s; ++i) {
        for (int j = 1; j <= s; ++j) {
            ret.set_element(i, j, gsl_matrix_get(&em.matrix, i-1, j-1));
        }
    }
    return ret;
}

gsl_matrix *Matrix::to_gsl_matrix() const {
    gsl_matrix *mat = gsl_matrix_alloc(_size, _size);
    for (int i = 1; i <= _size; ++i) {
        for (int j = 1; j <= _size; ++j) {
            gsl_matrix_set(mat, i-1, j-1, element(i, j));
        }
    }
    return mat;
}

Matrix Matrix::inverse() const {
    gsl_matrix *mat = to_gsl_matrix();
    gsl_permutation *p = gsl_permutation_alloc(_size);
    int s;
    gsl_linalg_LU_decomp(mat, p, &s);
    gsl_matrix *inv = gsl_matrix_alloc(_size, _size);
    gsl_linalg_LU_invert(mat, p, inv);
    gsl_permutation_free(p);
    Matrix ret(_size);
    for (int i = 0; i < _size; ++i) {
        for (int j = 0; j < _size; ++j) {
            ret.set_element(i+1, j+1, gsl_matrix_get(inv, i, j));
        }
    }
    gsl_matrix_free(mat);
    gsl_matrix_free(inv);
    return ret;
}

std::vector<std::pair<double,double> > Matrix::eigenvalues() const {
    std::vector<std::pair<double,double> > ret;
    gsl_matrix *mat = to_gsl_matrix();
    gsl_vector_complex *eval = gsl_vector_complex_alloc(_size);
    gsl_matrix_complex *evec = gsl_matrix_complex_alloc(_size, _size);
    gsl_eigen_nonsymmv_workspace *w = gsl_eigen_nonsymmv_alloc(_size);
    gsl_eigen_nonsymmv(mat, eval, evec, w);
    gsl_eigen_nonsymmv_free(w);
    gsl_eigen_nonsymmv_sort (eval, evec, GSL_EIGEN_SORT_ABS_DESC);
    for (int i = 0; i < _size; ++i) {
        gsl_complex eval_i = gsl_vector_complex_get(eval, i);
        ret.push_back(std::make_pair(GSL_REAL(eval_i), GSL_IMAG(eval_i)));
    }
    gsl_vector_complex_free(eval);
    gsl_matrix_complex_free(evec);
    gsl_matrix_free(mat);
    return ret;
}

std::ostream& operator <<(std::ostream &os, const Matrix &m) {
    int s = m.size();
    for (int i = 1; i <= s; ++i) {
        for (int j = 1; j <= s; ++j) {
            os << m.element(i, j);
            if (j < s)
                os << "\t";
        }
        os << std::endl;
    }
    return os;
}
