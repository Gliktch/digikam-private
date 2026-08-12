/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operation on items
 *
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QWidget>
#include <QUrl>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"

namespace Digikam
{
class Album;

class DIGIKAM_GUI_EXPORT ItemViewUtilities : public QObject
{
    Q_OBJECT

public:

    enum DeleteMode
    {
        DeletePermanently = 1,
        DeleteUseTrash    = 2
    };

public:

    explicit ItemViewUtilities(QWidget* const parentWidget);

public Q_SLOTS:

    void copyItemsToExternalFolder(const QList<ItemInfo>& infos);
    void createNewAlbumForInfos(const QList<ItemInfo>& infos, Album* currentAlbum);
    bool deleteImages(const QList<ItemInfo>& infos, const DeleteMode deleteMode);
    void deleteImagesDirectly(const QList<ItemInfo>& infos, const DeleteMode deleteMode);

    void insertToLightTableAuto(const QList<ItemInfo>& all, const QList<ItemInfo>& selected, const ItemInfo& current);
    void insertToLightTable(const QList<ItemInfo>& list, const ItemInfo& current, bool addTo);

    void insertToQueueManager(const QList<ItemInfo>& list, const ItemInfo& currentInfo, bool newQueue);
    void insertSilentToQueueManager(const QList<ItemInfo>& list, const ItemInfo& currentInfo, int queueid);

    void notifyFileContentChanged(const QList<QUrl>& urls);

    void openInfos(const ItemInfo& info, const QList<ItemInfo>& allInfosToOpen, Album* currentAlbum);
    void openInfosWithDefaultApplication(const QList<ItemInfo>& allInfosToOpen);

    void rename(const QUrl& imageUrl, const QString& newName, bool overwrite = false);
    void setAsAlbumThumbnail(Album* album, const ItemInfo& itemInfo);

    void createGroupByTimeFromInfoList(const ItemInfoList& itemInfoList);
    void createGroupByFilenameFromInfoList(const ItemInfoList& itemInfoList);
    void createGroupByTimelapseFromInfoList(const ItemInfoList& itemInfoList);

Q_SIGNALS:

    void editorCurrentUrlChanged(const QUrl& url);
    void signalImagesDeleted(const QList<qlonglong>& imageIds);

protected:

    QWidget* m_widget      = nullptr;
    int      m_lastAlbumId = -1;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemViewUtilities::DeleteMode)
