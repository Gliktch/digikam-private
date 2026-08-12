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

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QDebug>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

class KConfigGroup;

namespace DigikamGenericJAlbumPlugin
{

/**
 * This class stores all the export settings. It is initialized by the
 * Wizard and read by the Generator.
 */
class JAlbumSettings
{
public:

    enum ImageGetOption
    {
        ALBUMS = 0,
        IMAGES
    };

public:

    explicit JAlbumSettings(DInfoInterface* const iface = nullptr);
    ~JAlbumSettings() = default;

    /**
     * Read and write settings in config file between sessions.
     */
    void  readSettings(const KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

public:

    QString                   m_destPath;
    QString                   m_jalbumPath;          ///< jAlbum java archive path.
    QString                   m_javaPath;            ///< Java executable path.
    QString                   m_imageSelectionTitle; ///< Jalbum title to use for JAlbumSettings::ImageGetOption::IMAGES selection.

    ImageGetOption            m_getOption = IMAGES;  ///< Type of image selection (albums or images list).

    DInfoInterface::DAlbumIDs m_albumList;           ///< Albums list for ImageGetOption::ALBUMS selection.

    QList<QUrl>               m_imageList;           ///< Images list for ImageGetOption::IMAGES selection.

    DInfoInterface*           m_iface     = nullptr; ///< Interface to handle items information.
};

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const JAlbumSettings& t);

} // namespace DigikamGenericJAlbumPlugin
