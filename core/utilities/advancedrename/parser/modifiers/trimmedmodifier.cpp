/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : trimmed token modifier
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "trimmedmodifier.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

TrimmedModifier::TrimmedModifier()
    : Modifier(i18n("Trimmed"),
               i18n("Remove leading, trailing and extra whitespace"),
               QLatin1String("edit-cut"))
{
    QString token(QLatin1String("{trim}"));
    addToken(token, description());

    QRegularExpression reg(escapeToken(token));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString TrimmedModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& /*match*/)
{
    return settings.str2Modify.simplified();
}

} // namespace Digikam

#include "moc_trimmedmodifier.cpp"
