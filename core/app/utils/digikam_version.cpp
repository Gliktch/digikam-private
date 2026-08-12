/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-09
 * Description : digiKam release ID header.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikam_version.h"

// Qt includes

#include <QDate>
#include <QTime>
#include <QLocale>
#include <QSysInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_gitversion.h"
#include "digikam_builddate.h"
#include "digikam_globals.h"
#include "daboutdata.h"

namespace Digikam
{

int digiKamMakeIntegerVersion(int major, int minor, int patch)
{
    return (((major) << 16) | ((minor) << 8) | (patch));
}

const QString digiKamVersion()
{
    return QLatin1String(digikam_version);
}

const QDateTime digiKamBuildDate()
{
    QDateTime dt = QDateTime::fromString(QLatin1String(BUILD_DATE), QLatin1String("yyyyMMddTHHmmss"));

    return asDateTimeUTC(dt);
}

const QString additionalInformation()
{
    QString ret = QString::fromUtf8("<table>");

    ret        += QString::fromUtf8("<tr><td>%1</td><td>%2</td></tr>")
                  .arg(i18n("Support: "))
                  .arg(QString::fromLatin1("<a href='https://www.digikam.org/support/'>digikam.org/support </a>"));
    ret        += QString::fromUtf8("<tr><td>%1</td><td>%2</td></tr>")
                  .arg(i18n("Documentation: "))
                  .arg(QString::fromLatin1("<a href='https://www.digikam.org/documentation/'>digikam.org/documentation</a>"));
    ret        += QString::fromUtf8("<tr><td>%1</td><td>%2</td></tr>")
                  .arg(i18n("Contribute: "))
                  .arg(QString::fromLatin1("<a href='https://www.digikam.org/contribute/'>digikam.org/contribute</a>"));
    ret        += QString::fromUtf8("<tr><td>%1</td><td>%2</td></tr>")
                  .arg(i18n("Donate: "))
                  .arg(QString::fromLatin1("<a href='https://www.digikam.org/donate/'>digikam.org/donate</a>"));

    ret        += QString::fromUtf8("</table>");

    return ret;
}

} // namespace Digikam
