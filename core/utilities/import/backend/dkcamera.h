/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : abstract camera interface class
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QString>
#include <QStringList>
#include <QByteArray>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

class DMetadata;

class DKCamera : public QObject
{
    Q_OBJECT

public:

    enum CameraDriverType
    {
        GPhotoDriver = 0,
        UMSDriver
    };

public:

    explicit DKCamera(const QString& title, const QString& model, const QString& port, const QString& path);
    ~DKCamera() override = default;

public:

    virtual bool doConnect()                                                                                        = 0;
    virtual void cancel()                                                                                           = 0;

    virtual bool getFolders(const QString& folder)                                                                  = 0;

    /// If getImageDimensions is false, the camera shall set width and height to -1
    /// if the values are not immediately available
    virtual bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList)               = 0;
    virtual void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)   = 0;
    virtual bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)                    = 0;
    virtual bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta)                       = 0;
    virtual bool getFreeSpace(qint64& bytesSize, qint64& bytesAvail)                                                = 0;
    virtual bool getPreview(QImage& preview)                                                                        = 0;

    virtual bool capture(CamItemInfo& itemInfo)                                                                     = 0;

    virtual bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)              = 0;
    virtual bool deleteItem(const QString& folder, const QString& itemName)                                         = 0;
    virtual bool uploadItem(const QString& folder, const QString& itemName,
                            const QString& localFile, CamItemInfo& itemInfo)                                        = 0;
    virtual bool cameraSummary(QString& summary)                                                                    = 0;
    virtual bool cameraManual(QString& manual)                                                                      = 0;
    virtual bool cameraAbout(QString& about)                                                                        = 0;

    virtual bool setLockItem(const QString& folder, const QString& itemName, bool lock)                             = 0;

    virtual DKCamera::CameraDriverType cameraDriverType()                                                           = 0;
    virtual QByteArray                 cameraMD5ID()                                                                = 0;

public:

    QString title()                                                             const;
    QString model()                                                             const;
    QString port()                                                              const;
    QString path()                                                              const;
    QString uuid()                                                              const;

    bool    thumbnailSupport()                                                  const;
    bool    deleteSupport()                                                     const;
    bool    uploadSupport()                                                     const;
    bool    mkDirSupport()                                                      const;
    bool    delDirSupport()                                                     const;
    bool    captureImageSupport()                                               const;
    bool    captureImagePreviewSupport()                                        const;

    QString mimeType(const QString& fileext)                                    const;

    void printSupportedFeatures();

protected:

    void    fillItemInfoFromMetadata(CamItemInfo& item, const DMetadata& meta)  const;

protected:

    bool        m_thumbnailSupport              = false;
    bool        m_deleteSupport                 = false;
    bool        m_uploadSupport                 = false;
    bool        m_mkDirSupport                  = false;
    bool        m_delDirSupport                 = false;
    bool        m_captureImageSupport           = false;
    bool        m_captureImagePreviewSupport    = false;

    QStringList m_imageFilter;
    QStringList m_movieFilter;
    QStringList m_audioFilter;
    QStringList m_rawFilter;

    QString     m_model;
    QString     m_port;
    QString     m_path;
    QString     m_title;
    QString     m_uuid;

Q_SIGNALS:

    void signalFolderList(const QStringList&);

private:

    /// @note disabled
    DKCamera(QObject*) = delete;
};

} // namespace Digikam
