/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a class to calculate filter weights for hot pixels tool
 *
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixelsweights.h"

// C++ includes

#include <cstring>

// Qt includes

#include <QScopedArrayPointer>

namespace Digikam
{

HotPixelsWeights::HotPixelsWeights(const HotPixelsWeights& w)
{
    (*this) = w;
}

HotPixelsWeights::~HotPixelsWeights()
{
    if (!m_weightMatrices)
    {
        return;
    }

    for (int i = 0 ; i < m_positions.count() ; ++i)
    {
        for (unsigned int j = 0 ; j < m_height ; ++j)
        {
            delete [] m_weightMatrices[i][j];
        }
    }

    delete [] m_weightMatrices;
}

unsigned int HotPixelsWeights::height() const
{
    return m_height;
}

unsigned int HotPixelsWeights::polynomeOrder() const
{
    return m_polynomeOrder;
}

bool HotPixelsWeights::twoDim() const
{
    return m_twoDim;
}

unsigned int HotPixelsWeights::width() const
{
    return m_width;
}

void HotPixelsWeights::setHeight(int h)
{
    m_height = h;
}

void HotPixelsWeights::setPolynomeOrder(int order)
{
    m_polynomeOrder = order;
}

void HotPixelsWeights::setTwoDim(bool td)
{
    m_twoDim = td;
}

void HotPixelsWeights::setWidth(int w)
{
    m_width = w;
}

double** HotPixelsWeights::operator[](int n) const
{
    return m_weightMatrices[n];
}

const QList<QPoint> HotPixelsWeights::positions() const
{
    return m_positions;
}

int HotPixelsWeights::coefficientNumber() const
{
    return m_coefficientNumber;
}

double** * HotPixelsWeights::weightMatrices() const
{
    return m_weightMatrices;
}

HotPixelsWeights& HotPixelsWeights::operator=(const HotPixelsWeights& w)
{
    if (this == &w)
    {
        // we have to be sure that we are not self-assignment

        return *this;
    }

    m_height            = w.height();
    m_width             = w.width();
    m_positions         = w.positions();
    m_coefficientNumber = w.coefficientNumber();
    m_twoDim            = w.twoDim();
    m_polynomeOrder     = w.polynomeOrder();

    // Allocate memory and copy weights
    // if the original one was calculated

    if (!w.weightMatrices())
    {
        return *this;
    }
    else
    {
        double** * const origMatrices = w.weightMatrices();

        // Allocate m_positions.count() matrices

        m_weightMatrices              = new double** [m_positions.count()];

        for (int i = 0 ; i < m_positions.count() ; ++i)
        {
            // Allocate m_height rows on each position

            m_weightMatrices[i] = new double* [m_height];

            for (uint j = 0 ; j < m_height ; ++j)
            {
                // Allocate m_width columns on each row

                m_weightMatrices[i][j] = new double[m_width] { 0.0 };

                for (uint k = 0 ; k < m_width ; ++k)
                {
                    m_weightMatrices[i][j][k] = origMatrices[i][j][k];
                }
            }
        }
    }

    return *this;
}

void HotPixelsWeights::calculateHotPixelsWeights()
{

#ifndef __clang_analyzer__

    m_coefficientNumber = (m_twoDim ? ((size_t)m_polynomeOrder + 1) * ((size_t)m_polynomeOrder + 1)
                                    :  (size_t)m_polynomeOrder + 1);
    size_t  ix, iy, i, j;
    int     x, y;

    // Determine coordinates of pixels to be sampled

    if (m_twoDim)
    {

        int iPolynomeOrder = (int) m_polynomeOrder; // lets avoid signed/unsigned comparison warnings
        int iHeight        = (int) height();
        int iWidth         = (int) width();

        for (y = (-1)*iPolynomeOrder ; (y < iHeight + iPolynomeOrder) ; ++y)
        {
            for (x = (-1)*iPolynomeOrder ; (x < iWidth + iPolynomeOrder) ; ++x)
            {
                if (
                    ((x < 0)       && (y < 0)        && ((-x - y)                   < (iPolynomeOrder + 2)))            ||
                    ((x < 0)       && (y >= iHeight) && ((-x + y - iHeight)         < (iPolynomeOrder + 1)))            ||
                    ((x >= iWidth) && (y < 0)        && ((x - y - iWidth)           < (iPolynomeOrder + 1)))            ||
                    ((x >= iWidth) && (y >= iHeight) && ((x + y - iWidth - iHeight) < iPolynomeOrder))                  ||
                    ((x < 0)       && (y >= 0)       && (y < iHeight)) || ((x >= iWidth)  && (y >= 0) && (y < iHeight)) ||
                    ((y < 0)       && (x >= 0)       && (x < iWidth))  || ((y >= iHeight) && (x >= 0) && (x < iWidth))
                   )
                {
                    QPoint position(x,y);
                    m_positions.append(position);
                }
            }
        }
    }
    else
    {
        // In the one-dimensional case, only the y coordinate and y size is used.

        // cppcheck-suppress signConversion
        for (y = (-1)*m_polynomeOrder ; y < 0 ; ++y)
        {
            QPoint position(0, y);
            m_positions.append(position);
        }

        for (y = (int)height() ; (y < (int)height() + (int)m_polynomeOrder) ; ++y)
        {
            QPoint position(0, y);
            m_positions.append(position);
        }
    }

    // Allocate memory.

    QScopedArrayPointer<double> matrix (new double[m_coefficientNumber * m_coefficientNumber] { 0.0 });
    QScopedArrayPointer<double> vector0(new double[m_positions.count() * m_coefficientNumber] { 0.0 });
    QScopedArrayPointer<double> vector1(new double[m_positions.count() * m_coefficientNumber] { 0.0 });

    // Calculate coefficient matrix and vectors

    for (iy = 0 ; iy < m_coefficientNumber ; ++iy)
    {
        for (ix = 0 ; ix < m_coefficientNumber ; ++ix)
        {
            matrix[(int)(iy* m_coefficientNumber + ix)] = 0.0;
        }

        for (j = 0 ; j < (size_t)m_positions.count() ; ++j)
        {
            vector0[(int)(iy * m_positions.count() + j)] = polyTerm(iy, m_positions.at((int)j).x(),
                                                                    m_positions.at((int)j).y(), m_polynomeOrder);

            for (ix = 0 ; ix < m_coefficientNumber ; ++ix)
            {
                matrix[(int)(iy* m_coefficientNumber + ix)] += (vector0[(int)(iy * m_positions.count() + j)] *
                                                                polyTerm(ix, m_positions.at((int)j).x(), m_positions.at((int)j).y(), m_polynomeOrder));
            }
        }
    }

    // Invert matrix.

    matrixInv (matrix.data(), m_coefficientNumber);

    // Multiply inverse matrix with vector.

    for (iy = 0 ; iy < m_coefficientNumber ; ++iy)
    {
        for (j = 0 ; j < (size_t)m_positions.count() ; ++j)
        {
            vector1[(int)(iy * m_positions.count() + j)] = 0.0;

            for (ix = 0 ; ix < m_coefficientNumber ; ++ix)
            {
                vector1[(int)(iy * m_positions.count() + j)] += matrix[(int)(iy  * m_coefficientNumber + ix)] *
                                                                vector0[(int)(ix * m_positions.count() + j)];
            }
        }
    }

    // Store weights

    // Allocate m_positions.count() matrices.

    m_weightMatrices = new double** [m_positions.count()];

    for (i = 0 ; i < (size_t)m_positions.count() ; ++i)
    {
        // Allocate m_height rows on each position

        m_weightMatrices[i] = new double* [m_height];

        for (j = 0 ; j < m_height ; ++j)
        {
            // Allocate m_width columns on each row

            m_weightMatrices[i][j] = new double[m_width];
        }
    }

    for (y = 0 ; y < (int)m_height ; ++y)
    {
        for (x = 0 ; x < (int)m_width ; ++x)
        {
            for (j = 0 ; j < (size_t)m_positions.count() ; ++j)
            {
                m_weightMatrices[j][y][x] = 0.0;

                for (iy = 0 ; iy < m_coefficientNumber ; ++iy)
                {
                    m_weightMatrices[j][y][x] += vector1[(int)(iy * m_positions.count() + j)] *
                                                 polyTerm(iy, x, y, m_polynomeOrder);
                }

                m_weightMatrices[j][y][x] *= (double)m_positions.count();
            }
        }
    }

#endif

}

bool HotPixelsWeights::operator==(const HotPixelsWeights& ws) const
{
    return (
            (m_height        == ws.height())        &&
            (m_width         == ws.width())         &&
            (m_polynomeOrder == ws.polynomeOrder()) &&
            (m_twoDim        == ws.twoDim())
           );
}

/**
 * Invert a quadratic matrix.
 */
void HotPixelsWeights::matrixInv (double* const a, const size_t size)
{
    QScopedArrayPointer<double> b(new double[size * size] { 0.0 });
    size_t ix, iy, j;

    // Copy matrix to new location.

    memcpy(b.data(), a, sizeof (double) * size * size);

    // Set destination matrix to unit matrix.

    for (iy = 0 ; iy < size ; ++iy)
    {
        for (ix = 0 ; ix < size ; ++ix)
        {
            a[(int)(iy* size + ix)] = (ix == iy) ? 1.0 : 0.0;
        }
    }

    // Convert matrix to upper triangle form.

    for (iy = 0 ; iy < size - 1 ; ++iy)
    {
        for (j = iy + 1 ; j < size ; ++j)
        {
            const double factor = b[(int)(j * size + iy)] / b[(int)(iy * size + iy)];

            for (ix = 0 ; ix < size ; ++ix)
            {
                b[(int)(j* size + ix)] -= factor * b[(int)(iy * size + ix)];
                a[(int)(j* size + ix)] -= factor * a[(int)(iy * size + ix)];
            }
        }
    }

    // Convert matrix to diagonal form.

    for (iy = size - 1 ; iy > 0 ; --iy)
    {
        for (j = 0 ; j < iy ; ++j)
        {
            const double factor =  b[(int)(j * size + iy)] / b[(int)(iy * size + iy)];

            for (ix = 0 ; ix < size ; ++ix)
            {
                a[(int)(j* size + ix)] -= factor * a[(int)(iy * size + ix)];
            }
        }
    }

    // Convert matrix to unit matrix.

    for (iy = 0 ; iy < size ; ++iy)
    {
        for (ix = 0 ; ix < size ; ++ix)
        {
            a[(int)(iy* size + ix)] /= b[(int)(iy * size + iy)];
        }
    }
}

/**
 * Calculates one term of the polynomial
 */
double HotPixelsWeights::polyTerm (const size_t i_coeff, const int x, const int y, const int poly_order) const
{
    const size_t x_power = i_coeff / ((size_t)poly_order + 1);
    const size_t y_power = i_coeff % ((size_t)poly_order + 1);
    int result           = 1;
    size_t i;

    for (i = 0 ; i < x_power ; ++i)
    {
        result *= x;
    }

    for (i = 0 ; i < y_power ; ++i)
    {
        result *= y;
    }

    return (double)result;
}

} // namespace Digikam
