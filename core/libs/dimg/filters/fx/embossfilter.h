/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Emboss threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Emboss algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

class DIGIKAM_EXPORT EmbossFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit EmbossFilter(QObject* const parent = nullptr);
    explicit EmbossFilter(DImg* const orgImage, QObject* const parent = nullptr, int depth=30);
    ~EmbossFilter() override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:EmbossFilter");
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

    QString         filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                    override;
    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;
    void embossMultithreaded(uint start, uint stop, uint h, double Depth);

    inline int Lim_Max (int Now, int Up, int Max);
    inline int getOffset(int Width, int X, int Y, int bytesDepth);

private:

    int m_depth = 30;
};

} // namespace Digikam
