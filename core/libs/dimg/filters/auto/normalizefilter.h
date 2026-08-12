/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : normalize image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT NormalizeFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit NormalizeFilter(QObject* const parent = nullptr);
    NormalizeFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent = nullptr);
    ~NormalizeFilter()                                                        override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:NormalizeFilter");
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    static QString DisplayableName();

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;

    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;
    void normalizeImage();

private:

    struct Q_DECL_HIDDEN NormalizeParam
    {
        unsigned short* lut = nullptr;
        double          min = 0.0;
        double          max = 0.0;
    };

    DImg m_refImage;
};

} // namespace Digikam
