/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Charcoal threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

class DIGIKAM_EXPORT CharcoalFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit CharcoalFilter(QObject* const parent = nullptr);
    explicit CharcoalFilter(DImg* const orgImage,
                            QObject* const parent = nullptr,
                            double pencil = 5.0,
                            double smooth = 10.0);
    ~CharcoalFilter()                                                         override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:CharcoalFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;
    void            readParameters(const FilterAction& action)                override;

private:

    void filterImage()                                                        override;
    bool convolveImage(const unsigned int order, const double* kernel);
    int  getOptimalKernelWidth(double radius, double sigma);
    void convolveImageMultithreaded(uint start,
                                    uint stop,
                                    double* normal_kernel,
                                    double kernelWidth);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
