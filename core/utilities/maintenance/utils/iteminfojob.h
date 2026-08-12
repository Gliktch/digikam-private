/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-22-01
 * Description : interface to get item info from database.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QList>
#include <QByteArray>

// Local includes

#include "album.h"
#include "iteminfo.h"

namespace Digikam
{

class ItemInfoJob : public QObject
{
    Q_OBJECT

public:

    explicit ItemInfoJob();
    ~ItemInfoJob() override;

    void allItemsFromAlbum(Album* const album);
    void stop();
    bool isRunning() const;

Q_SIGNALS:

    void signalItemsInfo(const ItemInfoList& items);
    void signalCompleted();

private Q_SLOTS:

    void slotResult();
    void slotData(const QList<ItemListerRecord>& data);

private:

    /// @note disabled
    ItemInfoJob(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
