/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect enfuse binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "enfusebinary.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericExpoBlendingPlugin
{

double EnfuseBinary::getVersion() const
{
    return versionDouble;
}

bool EnfuseBinary::parseHeader(const QString& output)
{
    // Work around Enfuse <= 3.2
    // The output look like this : ==== enfuse, version 3.2 ====
    QString headerStartsOld = QLatin1String("==== enfuse, version ");
    QString firstLine = output.section(QLatin1Char('\n'), m_headerLine, m_headerLine);

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << path() << " help header line: \n" << firstLine;

    if      (firstLine.startsWith(m_headerStarts))
    {
        setVersion(firstLine.remove(0, m_headerStarts.length()));
        QStringList versionList = version().split(QLatin1Char('.'));
        versionList.removeLast();
        versionDouble = versionList.join(QLatin1Char('.')).toDouble();

        Q_EMIT signalEnfuseVersion(versionDouble);

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Found " << path() << " version: " << version();

        return true;
    }
    else if (firstLine.startsWith(headerStartsOld))
    {
        setVersion(firstLine.remove(0, headerStartsOld.length()));
        QStringList versionList = version().split(QLatin1Char('.'));
        versionList.removeLast();
        versionDouble = versionList.join(QLatin1Char('.')).toDouble();

        Q_EMIT signalEnfuseVersion(versionDouble);

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Found " << path() << " version: " << version();

        return true;
    }

    return false;
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_enfusebinary.cpp"
