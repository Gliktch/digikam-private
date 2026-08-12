/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : A modifier for appending a suffix number to a renaming option.
 *               This guarantees a unique string for duplicate values.
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "uniquemodifier.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

UniqueModifier::UniqueModifier()
    : Modifier(i18nc("unique value for duplicate strings", "Unique"),
               i18n("Add a suffix number to have unique strings in duplicate values"),
               QLatin1String("button_more"))
{
    addToken(QLatin1String("{unique}"),
             description());

    addToken(QLatin1String("{unique:||n||}"),
             i18n("Add a suffix number, ||n|| specifies the number of digits to use"));

    addToken(QLatin1String("{unique:||n||,||c||,||0||}"),
             i18n("Add a suffix number, "
                  "||n|| specifies the number of digits to use, "
                  "||c|| specifies the separator char before the numbers, "
                  "||a|| optional to include all options for uniqueness, "
                  "||0|| optional to always pad with ||n|| zero digits"));
             QRegularExpression reg(QLatin1String("\\{unique(:(\\d+))?(,([ -~]))?(,(a|0|a0|0a))?\\}"));

    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString UniqueModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    ParseResults::ResultsKey key = settings.currentResultsKey;
    QString tempStr2Modify;


    if (match.captured(6).contains(QLatin1Char('a')))
    {
        QFileInfo info(settings.fileUrl.toLocalFile());
        tempStr2Modify = settings.results.resultValuesAsString() + info.suffix();
    }
    else
    {
        tempStr2Modify = settings.str2Modify;
    }

    cache[key] << tempStr2Modify;
    bool notUnique = (cache[key].count(tempStr2Modify) > 1);

    if  (notUnique || (match.captured(6).contains(QLatin1Char('0'))))
    {
        QString result = settings.str2Modify;
        int index      = 0;

        if (notUnique)
        {
            index      = cache[key].count(tempStr2Modify) - 1;
        }

        bool ok        = true;
        int slength    = match.captured(2).toInt(&ok);
        QString sep    = match.captured(4);
        QString unique = sep.isEmpty() ? QString::fromUtf8("_%1")
                                       : sep + QString::fromUtf8("%1");
        slength        = (slength == 0 || !ok) ? 1 : slength;
        result        += unique.arg(index, slength, 10, QLatin1Char('0'));

        return result;
    }

    return settings.str2Modify;
}

void UniqueModifier::reset()
{
    cache.clear();
}

} // namespace Digikam

#include "moc_uniquemodifier.cpp"
