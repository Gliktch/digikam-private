/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "dkcamera.h"

namespace Digikam
{

class DMetadata;

/**
 * USB Mass Storage camera Implementation of abstract type DKCamera
 */
class UMSCamera : public DKCamera
{
    Q_OBJECT

public:

    explicit UMSCamera(const QString& title, const QString& model, const QString& port, const QString& path);
    ~UMSCamera()                                                                                                    override = default;

    QByteArray                 cameraMD5ID()                                                                        override;
    DKCamera::CameraDriverType cameraDriverType()                                                                   override;

    bool doConnect()                                                                                                override;
    void cancel()                                                                                                   override;

    bool getFolders(const QString& folder)                                                                          override;
    bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList)                       override;
    void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)           override;

    bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)                            override;
    bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta)                               override;

    bool setLockItem(const QString& folder, const QString& itemName, bool lock)                                     override;

    bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)                      override;
    bool deleteItem(const QString& folder, const QString& itemName)                                                 override;
    bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& info)    override;

    bool cameraSummary(QString& summary)                                                                            override;
    bool cameraManual(QString& manual)                                                                              override;
    bool cameraAbout(QString& about)                                                                                override;

    bool getFreeSpace(qint64& bytesSize, qint64& bytesAvail)                                                        override;

    bool getPreview(QImage& preview)                                                                                override;
    bool capture(CamItemInfo& itemInfo)                                                                             override;

private:

    /**
     * Try to find UUID of UMS camera media using Solid interface.
     * Search use mount path as reference. Return UUID as string
     * else an empty string
     */
    void getUUIDFromSolid();

private:

    bool m_cancel = false;

private:

    /// @note disabled
    explicit UMSCamera(QObject*) = delete;
};

} // namespace Digikam
