/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "jalbumsettings.h"

// KDE includes

#include <kconfiggroup.h>

namespace DigikamGenericJAlbumPlugin
{

JAlbumSettings::JAlbumSettings(DInfoInterface* const iface)
{
    m_iface       = iface;

    QString dfltAlbumPath;

#ifdef Q_OS_WIN

    dfltAlbumPath = QLatin1String(qgetenv("HOMEDRIVE").constData());
    dfltAlbumPath.append(QLatin1String(qgetenv("HOMEPATH").constData()));
    dfltAlbumPath.append(QLatin1String("\\Documents\\My Albums"));

#else

    dfltAlbumPath = QLatin1String(qgetenv("HOME").constData());
    dfltAlbumPath.append(QLatin1String("/Documents/My Albums"));

#endif

    m_destPath = dfltAlbumPath;
}

void JAlbumSettings::readSettings(const KConfigGroup& group)
{
    m_destPath            = group.readEntry("destPath",                 QString());
    m_jalbumPath          = group.readEntry("jalbumPath",               QString());
    m_javaPath            = group.readEntry("javaPath",                 QString());
    m_imageSelectionTitle = group.readEntry("imageSelectionTitle",     QString());
    m_getOption           = (ImageGetOption)group.readEntry("SelMode", (int)IMAGES);
}

void JAlbumSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("destPath",            m_destPath);
    group.writeEntry("jalbumPath",          m_jalbumPath);
    group.writeEntry("javaPath",            m_javaPath);
    group.writeEntry("imageSelectionTitle", m_imageSelectionTitle);
    group.writeEntry("SelMode",             (int)m_getOption);
}

QDebug operator<<(QDebug dbg, const JAlbumSettings& t)
{
    dbg.nospace() << "JAlbumSettings::Items: "
                  << t.m_imageList << ", ";
    dbg.nospace() << "JAlbumSettings::DestPath: "
                  << t.m_destPath;
    dbg.nospace() << "JAlbumSettings::JalbumPath: "
                  << t.m_jalbumPath;
    dbg.nospace() << "JAlbumSettings::JavaPath: "
                  << t.m_javaPath;
    dbg.nospace() << "JAlbumSettings::ImageSelectionTitle: "
                  << t.m_imageSelectionTitle;
    return dbg.space();
}

} // namespace DigikamGenericJAlbumPlugin
