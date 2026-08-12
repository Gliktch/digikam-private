/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : metadatahub manager
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPointer>

namespace Digikam
{

class ItemInfo;

class MetadataHubMngr : public QObject
{
    Q_OBJECT

public:

    static MetadataHubMngr* instance();
    ~MetadataHubMngr() override;

    static QPointer<MetadataHubMngr> internalPtr;
    static bool                      isCreated();

    void addPendingIds(const QList<qlonglong>& imageIds);
    void addPending(const ItemInfo& info);
    void requestShutDown();

Q_SIGNALS:

    void signalPendingMetadata(int numbers);

public Q_SLOTS:

    void slotRemovePending(const ItemInfo& info);
    void slotApplyPending();

private:

    /// @note disabled
    MetadataHubMngr();
    explicit MetadataHubMngr(QObject*);

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
