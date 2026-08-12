/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-24
 * Description : Web Service settings container.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QSettings>

// Local includes

#include "digikam_export.h"
#include "o0settingsstore.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT WSSettings : public QObject
{
    Q_OBJECT

public:

    /// Images selection mode
    enum Selection
    {
        EXPORT = 0,
        IMPORT
    };

    enum WebService
    {
        FLICKR = 0,
        DROPBOX,
        IMGUR,
        FACEBOOK,
        SMUGMUG,
        GDRIVE,
        GPHOTO
    };

    enum ImageFormat
    {
        JPEG = 0,
        PNG
    };

public:

    explicit WSSettings(QObject* const parent = nullptr);
    ~WSSettings() override;

    ///@{
    /// Read and write settings in config file between sessions.
    void  readSettings(const KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);
    ///@}

    QString format() const;

    ///@{
    /// Helper methods to fill settings from GUI.
    static QMap<WebService,  QString> webServiceNames();
    static QMap<ImageFormat, QString> imageFormatNames();
    ///@}

    /// Helper method to list all user accounts (of all web service) that user logged in before.
    QStringList allUserNames(const QString& serviceName);

public:

    Selection           selMode             = EXPORT;           ///< Items selection mode

    QList<QUrl>         inputImages;                            ///< Selected items to upload.

    bool                addFileProperties   = false;
    bool                imagesChangeProp    = false;

    bool                removeMetadata      = false;

    int                 imageCompression    = 75;

    qint64              attLimitInMbytes    = 17;

    WebService          webService          = FLICKR;

    QString             userName;

    QSettings*          oauthSettings       = nullptr;
    O0SettingsStore*    oauthSettingsStore  = nullptr;

    QString             currentAlbumId;                         ///< Selected album to upload to

    int                 imageSize           = 1024;

    ImageFormat         imageFormat         = JPEG;

    QMap<QUrl, QUrl>    itemsList;                              ///< Map of original item and attached item (can be resized).
};

} // namespace Digikam
