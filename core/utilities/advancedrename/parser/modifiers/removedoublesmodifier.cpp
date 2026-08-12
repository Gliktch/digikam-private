/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a modifier for deleting duplicate words
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "removedoublesmodifier.h"

// Qt includes

#include <QSet>
#include <QString>
#include <QStringList>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

RemoveDoublesModifier::RemoveDoublesModifier()
    : Modifier(i18n("Remove Doubles"),
               i18n("Remove duplicate words"),
               QLatin1String("edit-copy"))
{
    addToken(QLatin1String("{removedoubles}"), description());

    QRegularExpression reg(QLatin1String("\\{removedoubles\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString RemoveDoublesModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& /*match*/)
{
    QString result    = settings.str2Modify;

    QSet<QString> knownWords;
    QStringList words = result.split(QLatin1Char(' '));
    QStringList newString;

    for (const QString& word : std::as_const(words))
    {
        if (!knownWords.contains(word))
        {
            knownWords.insert(word);
            newString << word;
        }
    }

    if (!newString.isEmpty())
    {
        result = newString.join(QLatin1Char(' '));
    }

    return result;
}

} // namespace Digikam

#include "moc_removedoublesmodifier.cpp"
