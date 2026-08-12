/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpfind binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cpfindbinary.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericPanoramaPlugin
{

CPFindBinary::CPFindBinary(QObject* const parent)
    : DBinaryIface(QLatin1String("cpfind"),
                   QLatin1String("2010.4"),
                   QString(),
                   0,
                   QLatin1String("Hugin"),
                   QLatin1String("https://hugin.sourceforge.net/download/"),
                   QLatin1String("Panorama"),
                   QStringList(QLatin1String("--version"))),
      headerRegExp(QLatin1String("^Hugin'?s cpfind( Pre-Release)? (\\d+\\.\\d+(\\.\\d+)?)(\\D.*)?$"))
{
    Q_UNUSED(parent);

    setup();
}

bool CPFindBinary::parseHeader(const QString& output)
{
    QStringList lines    = output.split(QLatin1Char('\n'));
    m_developmentVersion = false;

    for (const QString& line : std::as_const(lines))
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << path() << " help header line: \n" << line;
        QRegularExpressionMatch regMatch = headerRegExp.match(line);

        if (regMatch.hasMatch())
        {
            m_version = regMatch.captured(2);

            if (!regMatch.captured(1).isEmpty())
            {
                m_developmentVersion = true;
            }

            return true;
        }

        m_developmentVersion = true;
    }

    return false;
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_cpfindbinary.cpp"
