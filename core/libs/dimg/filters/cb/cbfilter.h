/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-18
 * Description : color balance filter
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class DImg;

class DIGIKAM_EXPORT CBContainer
{

public:

    CBContainer()  = default;
    ~CBContainer() = default;

public:

    double red   = 1.0;
    double green = 1.0;
    double blue  = 1.0;
    double alpha = 1.0;
    double gamma = 1.0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT CBFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit CBFilter(QObject* const parent = nullptr);
    explicit CBFilter(DImg* const orgImage,
                      QObject* const parent = nullptr,
                      const CBContainer& settings = CBContainer());
    explicit CBFilter(const CBContainer& settings,
                      DImgThreadedFilter* const master,
                      const DImg& orgImage,
                      DImg& destImage,
                      int progressBegin = 0,
                      int progressEnd = 100);
    ~CBFilter()                                                       override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:ColorBalanceFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    void  readParameters(const FilterAction& action)                  override;

    QString         filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                    override;

private:

    void filterImage()                                                override;

    void reset();
    void setGamma(double val);
    void setTables(int* const redMap, int* const greenMap, int* const blueMap, int* const alphaMap, bool sixteenBit);
    void getTables(int* const redMap, int* const greenMap, int* const blueMap, int* const alphaMap, bool sixteenBit);
    void adjustRGB(double r, double g, double b, double a, bool sixteenBit);
    void applyCBFilter(const DImg& image, double r, double g, double b, double a);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
