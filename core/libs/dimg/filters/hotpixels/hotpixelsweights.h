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

#pragma once

// Qt includes

#include <QList>
#include <QRect>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT HotPixelsWeights
{

public:

    HotPixelsWeights() = default;
    HotPixelsWeights(const HotPixelsWeights& w);
    ~HotPixelsWeights();

    HotPixelsWeights& operator=(const HotPixelsWeights& w);
    bool operator==(const HotPixelsWeights& ws) const;
    double** operator[](int n)                  const;

    unsigned int height()                       const;
    void setHeight(int h);

    unsigned int polynomeOrder()                const;
    void setPolynomeOrder(int order);

    bool twoDim() const;
    void setTwoDim(bool td);

    void setWidth(int w);
    unsigned int width()                        const;

    const QList<QPoint> positions()             const;

    void calculateHotPixelsWeights();

protected:

    int coefficientNumber()                     const;

    double** * weightMatrices()                 const;

private:

    double polyTerm(const size_t i_coeff,
                    const int x,
                    const int y,
                    const int poly_order)       const;

    void   matrixInv(double* const a,
                     const size_t size);

private:

    unsigned int  m_height              = 0;
    unsigned int  m_width               = 0;
    unsigned int  m_coefficientNumber   = 0;
    bool          m_twoDim              = false;
    unsigned int  m_polynomeOrder       = 0;
    double** *    m_weightMatrices      = nullptr;     ///< Stores a list of weight matrices
    QList<QPoint> m_positions;
};

} // namespace Digikam
