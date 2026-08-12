/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date   : 2005-05-25
 * Description : Infrared threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT InfraredContainer
{

public:

    InfraredContainer()  = default;
    ~InfraredContainer() = default;

public:

    /// Sensibility: 200..2600 ISO
    int    sensibility  = 200;

    double redGain      = 0.4;
    double greenGain    = 2.1;
    double blueGain     = -0.8;
};

// ---------------------------------------------------------------------------

class DIGIKAM_EXPORT InfraredFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit InfraredFilter(QObject* const parent = nullptr);
    explicit InfraredFilter(DImg* const orgImage,
                            QObject* const parent = nullptr,
                            const InfraredContainer& settings = InfraredContainer());
    ~InfraredFilter()                                                         override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:InfraredFilter");
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
    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;

    inline int intMult8(uint a, uint b);
    inline int intMult16(uint a, uint b);

private:

    InfraredContainer m_settings;
};

} // namespace Digikam
