/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Change tonality image filter
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

class DIGIKAM_EXPORT TonalityContainer
{

public:

    TonalityContainer()  = default;
    ~TonalityContainer() = default;

public:

    int redMask     = 0;
    int greenMask   = 0;
    int blueMask    = 0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT TonalityFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit TonalityFilter(QObject* const parent = nullptr);
    explicit TonalityFilter(DImg* const orgImage,
                            QObject* const parent = nullptr,
                            const TonalityContainer& settings = TonalityContainer());
    ~TonalityFilter()                                       override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:TonalityFilter");
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

    QString filterIdentifier()                        const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                             override;
    void readParameters(const FilterAction& action)         override;

private:

    void filterImage()                                      override;

private:

    TonalityContainer m_settings;
};

} // namespace Digikam
