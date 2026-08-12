/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : finger-prints generator
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class QImage;

namespace Digikam
{

class ItemInfo;

class FingerPrintsGenerator : public MaintenanceTool
{
    Q_OBJECT

public:

    /**
     * @brief Constructor using AlbumList as argument. If list is empty, whole Albums collection is processed.
     */
    explicit FingerPrintsGenerator(const bool rebuildAll,
                                   const AlbumList& list = AlbumList(),
                                   ProgressItem* const parent = nullptr);
    ~FingerPrintsGenerator() override;

    void setUseMultiCoreCPU(bool b) override;

Q_SIGNALS:

    void signalScanNotification(const QString& msg, int type);

private:

    void processOne();
    void calculateAffectedAlbums();

private Q_SLOTS:

    void slotStart()                override;
    void slotDone()                 override;      // cppcheck-suppress virtualCallInConstructor
    void slotCancel()               override;
    void slotAffectedAlbumsFinished();
    void slotAdvance(const ItemInfo&, const QImage&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
