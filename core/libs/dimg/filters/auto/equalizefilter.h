/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : equalize image filter.
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

class DIGIKAM_EXPORT EqualizeFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit EqualizeFilter(QObject* const parent = nullptr);
    EqualizeFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent = nullptr);
    ~EqualizeFilter()                                                         override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:EqualizeFilter");
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
    void equalizeImage();

private:

    struct Q_DECL_HIDDEN double_packet
    {
        double_packet() = default;

        double red   = 0.0;
        double green = 0.0;
        double blue  = 0.0;
        double alpha = 0.0;
    };

    struct Q_DECL_HIDDEN int_packet
    {
        int_packet() = default;

        unsigned int red    = 0;
        unsigned int green  = 0;
        unsigned int blue   = 0;
        unsigned int alpha  = 0;
    };

    DImg m_refImage;
};

} // namespace Digikam
