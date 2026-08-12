/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : image quality sorter maintenance tool
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "album.h"
#include "maintenancetool.h"
#include "imagequalitysettings.h"

class QImage;

namespace Digikam
{

class ItemInfo;

class ImageQualitySorter : public MaintenanceTool
{
    Q_OBJECT

public:

    /**
     * @brief Constructor using ImageQualitySettings as argument including AlbumList. If list is empty, whole Albums collection is processed.
     */
    explicit ImageQualitySorter(const ImageQualitySettings& quality,
                                ProgressItem* const parent = nullptr);
    /**
     * @brief Constructor using QStringList and ImageQualitySettings as argument. If list is empty, whole Albums collection is processed.
     */
    ImageQualitySorter(const QStringList& itemPaths,
                       const ImageQualitySettings& quality,
                       ProgressItem* const parent = nullptr);
    ~ImageQualitySorter() override;

    void init(const ImageQualitySettings& quality);
    void setUseMultiCoreCPU(bool b) override;

private:

    void processOne();
    void calculateAffectedAlbums();

private Q_SLOTS:

    void slotStart()                override;
    void slotCancel()               override;
    void slotDone()                 override;             // cppcheck-suppress virtualCallInConstructor
    void slotAffectedAlbumsFinished();
    void slotAdvance(const ItemInfo&, const QImage&, int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
