/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : TextureFilter threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "texturecontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT TextureFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit TextureFilter(QObject* const parent = nullptr);
    explicit TextureFilter(DImg* const orgImage,
                           QObject* const parent = nullptr,
                           const TextureContainer& settings = TextureContainer());

    ~TextureFilter()                                                          override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:TextureFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 2;
    }

    static int              CurrentVersion()
    {
        return 2;
    }

    QString                 filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction            filterAction()                                    override;
    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;

private:

    TextureContainer m_settings;
};

} // namespace Digikam
