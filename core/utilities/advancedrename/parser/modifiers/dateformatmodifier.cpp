/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-10-26
 * Description : a modifier for recognize and format a date
 *
 * SPDX-FileCopyrightText: 2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dateformatmodifier.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "timeadjustcontainer.h"

namespace Digikam
{

DateFormatModifier::DateFormatModifier()
    : Modifier(i18n("Date Format"),
               i18n("Recognize and format a date"),
               QLatin1String("view-calendar"))
{
    addToken(QLatin1String("{dateformat:||yyyyMMdd||}"),
             i18n("Recognize and format a date to string") +
             QLatin1String(" (") + getDateFormatLinkText() + QLatin1Char(')'));

    QRegularExpression reg(QLatin1String("\\{dateformat(:(.*))?\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString DateFormatModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString result;
    QString format = match.captured(1).remove(0, 1);

    if (format.isEmpty())
    {
        return result;
    }

    TimeAdjustContainer tac;
    QDateTime dateTime = tac.getDateTimeFromString(settings.str2Modify);

    if (!dateTime.isValid())
    {
        return result;
    }

    result = dateTime.toString(format);

    return result;
}

} // namespace Digikam

#include "moc_dateformatmodifier.cpp"
