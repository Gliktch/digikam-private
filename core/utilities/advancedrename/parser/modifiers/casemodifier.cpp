/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : modifier to change the case of a renaming option
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "casemodifier.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

CaseModifier::CaseModifier()
    : Modifier(i18n("Change Case"), i18n("change the case of a renaming option"))
{
    setUseTokenMenu(true);

    addToken(QLatin1String("{upper}"),      i18n("Convert to uppercase"),
             i18n("Uppercase"));

    addToken(QLatin1String("{lower}"),      i18n("Convert to lowercase"),
             i18n("Lowercase"));

    addToken(QLatin1String("{firstupper}"), i18n("Convert the first letter of each word to uppercase"),
             i18n("First Letter of Each Word Uppercase"));

    QRegularExpression reg(QLatin1String("\\{(firstupper|lower|upper)\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString CaseModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    const QString& token = match.captured(1);

    if      (token == QLatin1String("firstupper"))
    {
        return firstupper(settings.str2Modify);
    }
    else if (token == QLatin1String("upper"))
    {
        return settings.str2Modify.toUpper();
    }
    else if (token == QLatin1String("lower"))
    {
        return settings.str2Modify.toLower();
    }

    return settings.str2Modify;
}

QString CaseModifier::firstupper(const QString& str2Modify)
{
    // cppcheck-suppress knownConditionTrueFalse
    if (str2Modify.isNull() || str2Modify.isEmpty())
    {
        return QString();
    }

    QString result = str2Modify.toLower();

    if (result.at(0).isLetter())
    {
        result[0] = result.at(0).toUpper();
    }

    for (int i = 0 ; i < result.length() - 1 ; ++i)
    {
        if (result.at(i + 1).isLetter() && !result.at(i).isLetter())
        {
            result[i + 1] = result.at(i + 1).toUpper();
        }
    }

    return result;
}

} // namespace Digikam

#include "moc_casemodifier.cpp"
