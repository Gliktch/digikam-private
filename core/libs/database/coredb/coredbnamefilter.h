/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-02
 * Description : Core database file name filters based on file suffixes.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>
#include <QRegularExpression>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbNameFilter
{
public:

    /**
     * Creates a name filter object with the given filter string.
     * The string is a list of text parts of which one needs to match
     * (file suffixes),
     * separated by ';' characters.
     */
    explicit CoreDbNameFilter(const QString& filter);

    /**
     * Returns if the specified name matches this filter
     */
    bool matches(const QString& name);

protected:

    QList<QRegularExpression> m_filterList;
};

} // namespace Digikam
