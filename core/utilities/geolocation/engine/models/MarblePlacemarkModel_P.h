/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QChar>
#include <QRegularExpression>
#include <QString>

namespace Marble
{

namespace GeoString
{
static const QRegularExpression combiningDiacriticalMarks(QString::fromUtf8("[\\x0300-\\x036F]+"));

QString deaccent(const QString& accentString)
{
    QString    result;

    result = accentString.normalized(QString::NormalizationForm_D).remove(combiningDiacriticalMarks);
    result.replace(QChar(0x00F8), QLatin1Char('o'));
    result.replace(QChar(0x0142), QLatin1Char('l'));
    return result;
}

} // namespace GeoString

} // namespace Marble
