/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : Interface for version file naming
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionnamingscheme.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

QVariant DefaultVersionNamingScheme::initialCounter()
{
    // start with _v1

    return 1;
}

QVariant DefaultVersionNamingScheme::incrementedCounter(const QVariant& counter)
{
    return counter.toInt() + 1;
}

QString DefaultVersionNamingScheme::baseName(const QString& currentPath, const QString& fileName,
                                             QVariant* counter, QVariant* intermediateCounter)
{
    Q_UNUSED(currentPath);

    // Perl: /^(.+?)(_v\d+?)?(-\d+?)?\.([^\.]+?)$/
    // But setMinimal() cannot replace Perl's non-greedy quantifiers, so we need three regexps

    int index                = fileName.lastIndexOf(QLatin1Char('.'));
    QString completeBaseName = (index == -1) ? fileName : fileName.left(index);

    // DSC000636_v5-3.JPG: intermediate

    QRegularExpression versionIntermediate(QRegularExpression::anchoredPattern(QLatin1String("(.+)_v(\\d+)-(\\d+)")));
    QRegularExpressionMatch match = versionIntermediate.match(completeBaseName);

    if (match.hasMatch())
    {
        if (counter)
        {
            *counter = match.captured(2).toInt();
        }

        if (intermediateCounter)
        {
            *intermediateCounter = match.captured(3).toInt();
        }

        return match.captured(1);
    }

    // DSC000636_v5.JPG: version

    QRegularExpression version(QRegularExpression::anchoredPattern(QLatin1String("(.+)_v(\\d+)")));
    QRegularExpressionMatch versionMatch = version.match(completeBaseName);
    if (versionMatch.hasMatch())
    {
        if (counter)
        {
            *counter = versionMatch.captured(2).toInt();
        }

        return versionMatch.captured(1);
    }

    // DSC000636.JPG: original file or different naming scheme

    return completeBaseName;
}

QString DefaultVersionNamingScheme::versionFileName(const QString& currentPath,
                                                    const QString& baseName, const QVariant& counter)
{
    Q_UNUSED(currentPath);

    return QString::fromUtf8("%1_v%2").arg(baseName).arg(counter.toInt());
}

QString DefaultVersionNamingScheme::intermediateFileName(const QString& currentPath, const QString& baseName,
                                                         const QVariant& version, const QVariant& counter)
{
    Q_UNUSED(currentPath);

    return QString::fromUtf8("%1_v%2-%3").arg(baseName).arg(version.toInt()).arg(counter.toInt());
}

QString DefaultVersionNamingScheme::directory(const QString& currentPath, const QString& fileName)
{
    Q_UNUSED(fileName);

    return currentPath;
}

QString DefaultVersionNamingScheme::intermediateDirectory(const QString& currentPath, const QString& fileName)
{
    Q_UNUSED(fileName);

    return currentPath;
}

} // namespace Digikam
