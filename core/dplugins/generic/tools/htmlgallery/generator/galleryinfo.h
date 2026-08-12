/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "galleryconfig.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * This class stores all the export settings. It is initialized by the
 * Wizard and read by the Generator.
 */
class GalleryInfo : public GalleryConfig
{
    Q_OBJECT

public:

    enum ImageGetOption
    {
        ALBUMS = 0,
        IMAGES
    };

public:

    explicit GalleryInfo(DInfoInterface* const iface = nullptr);
    ~GalleryInfo() override = default;

    QString fullFormatString()                                  const;

    QString thumbnailFormatString()                             const;

    QString getThemeParameterValue(const QString& theme, const QString& parameter,
                                   const QString& defaultValue) const;

    void setThemeParameterValue(const QString& theme, const QString& parameter,
                                const QString& value);

public:

    ImageGetOption            m_getOption   = IMAGES;       ///< Type of image selection (albums or images list).

    DInfoInterface::DAlbumIDs m_albumList;                  ///< Albums list for ImageGetOption::ALBUMS selection.

    QList<QUrl>               m_imageList;                  ///< Images list for ImageGetOption::IMAGES selection.

    DInfoInterface*           m_iface       = nullptr;      ///< Interface to handle items information.

private:

    /**
     * This method returns the string associated to the enum value.
     */
    QString getEnumString(const QString& itemName) const;
};

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const GalleryInfo& t);

} // namespace DigikamGenericHtmlGalleryPlugin
