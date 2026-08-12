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

#include "coredbnamefilter.h"

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CoreDbNameFilter::CoreDbNameFilter(const QString& filter)
{
    if (filter.isEmpty())
    {
        return;
    }

    QLatin1Char sep(';');
    int i = filter.indexOf(sep);

    if ((i == -1) && (filter.indexOf(QLatin1Char(' ')) != -1))
    {
        sep = QLatin1Char(' ');
    }

    QStringList list               = filter.split(sep, Qt::SkipEmptyParts);
    QStringList::const_iterator it = list.constBegin();

    while (it != list.constEnd())
    {
        QString pattern = QRegularExpression::wildcardToRegularExpression((*it).trimmed());
        pattern.replace(QLatin1String("[^/\\\\]"), QLatin1String(".")); // Windows
        pattern.replace(QLatin1String("[^/]"),     QLatin1String(".")); // Linux
        QRegularExpression wildcard(pattern, QRegularExpression::CaseInsensitiveOption);
        m_filterList << wildcard;
        ++it;
    }
}

bool CoreDbNameFilter::matches(const QString& name)
{
    QList<QRegularExpression>::const_iterator rit = m_filterList.constBegin();

    while (rit != m_filterList.constEnd())
    {
        if ((*rit).match(name).hasMatch())
        {
            return true;
        }

        ++rit;
    }

    return false;
}

} // namespace Digikam
