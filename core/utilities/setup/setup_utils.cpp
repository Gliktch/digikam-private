/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : utilities for digiKam setup
 *
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setup_p.h"

namespace Digikam
{

QStringList Setup::cleanUserFilterString(const QString& filter,
                                  const bool caseSensitive,
                                  const bool useSemicolon)
{
    QString filterString = filter;

    if (!caseSensitive)
    {
        filterString = filterString.toLower();
    }

    QChar separator;

    if (!useSemicolon)
    {
        separator = QLatin1Char(' ');
        filterString.remove(QLatin1Char('*'));
        filterString.replace(QLatin1Char(';'),     QLatin1Char(' '));
        filterString.replace(QLatin1String(" ."),  QLatin1String(" "));
        filterString.replace(QLatin1String(" -."), QLatin1String(" -"));
    }
    else
    {
        separator = QLatin1Char(';');
        filterString.replace(QLatin1String(";."),  QLatin1String(";"));
        filterString.replace(QLatin1String(";-."), QLatin1String(";-"));
    }

    QStringList filterList;
    const auto filters = filterString.split(separator, Qt::SkipEmptyParts);

    for (const QString& flt : filters)
    {
        filterList << flt.trimmed();
    }

    return filterList;
}

} // namespace Digikam
