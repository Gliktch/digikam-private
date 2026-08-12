/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : HotPixel properties container.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRect>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT HotPixelProps
{

public:

    QRect rect;
    int   luminosity = 0;

public:

    int y()                                                     const;
    int x()                                                     const;
    int width()                                                 const;
    int height()                                                const;

    bool operator==(const HotPixelProps& p)                     const;

    /// Helper methods to stream container data with string.

    QString toString()                                          const;
    bool fromString(const QString& str);

public:

    /// Helper methods to stream list of containers data with string list.

    static QStringList toStringList(const QList<HotPixelProps>& lst);
    static QList<HotPixelProps> fromStringList(const QStringList& hplst);

private:

    bool diagonal(const QRect& r1, const QRect& r2)             const;
};

} // namespace Digikam
