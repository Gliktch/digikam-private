/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect jAlbum jar archive
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "jalbumjar.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericJAlbumPlugin
{

JalbumJar::JalbumJar(QObject* const)
    : DBinaryIface(QLatin1String("JAlbum.jar"),
                   QLatin1String("jAlbum"),
                   QLatin1String("https://jalbum.net/"),
                   QLatin1String("jAlbum Export"),
                   QStringList(QLatin1String("-version")),  // TODO: to check version with "java -jar " prefix
                   i18n("jAlbum Gallery Generator."))
{
    setup();
}

bool JalbumJar::checkDirForPath(const QString& possibleDir)
{
    QString possiblePath = path(possibleDir);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Testing " << possiblePath << "...";

    bool ret = QFile::exists(possiblePath);

    if (ret)
    {
        m_isFound = true;
        m_pathDir = possibleDir;
        writeConfig();

        qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path();
    }

    Q_EMIT signalBinaryValid();

    return ret;
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumjar.cpp"
