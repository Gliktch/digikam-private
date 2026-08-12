/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : Albums history manager.
 *
 * SPDX-FileCopyrightText: 2004      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QObject>
#include <QStringList>

// Local includes

#include "labelstreeview.h"
#include "iteminfo.h"

namespace Digikam
{

class Album;
class HistoryItem;
class HistoryPosition;

/**
 * Manages the history of the last visited albums.
 *
 * The user is able to navigate through the albums, he has
 * opened during a session.
 */
class AlbumHistory : public QObject
{
    Q_OBJECT

public:

    explicit AlbumHistory(QObject* const parent = nullptr);
    ~AlbumHistory()                                   override;

    void addAlbums(const QList<Album*>& albums,
                   QWidget* const widget = nullptr);

    void addAlbums(const QList<Album*>& albums,
                   QWidget* const widget,
                   const QHash<LabelsTreeView::Labels, QList<int> >& selectedLabels);

    void deleteAlbum(Album* const album);
    void clearHistory();

    void back(QList<Album*>& album,
              QWidget** const widget,
              unsigned int steps = 1);

    void forward(QList<Album*>& album,
                 QWidget** const widget,
                 unsigned int steps = 1);

    void getCurrentAlbum(Album** const album,
                         QWidget** const widget) const;

    void getBackwardHistory(QStringList& list)   const;
    void getForwardHistory(QStringList& list)    const;

    bool isForwardEmpty()                        const;
    bool isBackwardEmpty()                       const;

    QHash<LabelsTreeView::Labels, QList<int> > neededLabels();

Q_SIGNALS:

    void signalSetCurrent(qlonglong imageId);
    void signalSetSelectedInfos(const QList<ItemInfo>&);

public Q_SLOTS:

    void slotAlbumCurrentChanged();
    void slotAlbumDeleted(Album* album);
    void slotAlbumsCleared();
    void slotAlbumSelected();
    void slotClearSelectPAlbum(const ItemInfo& imageInfo);
    void slotClearSelectTAlbum(int id);
    void slotCurrentChange(const ItemInfo& info);
    void slotImageSelected(const ItemInfoList& selectedImage);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
