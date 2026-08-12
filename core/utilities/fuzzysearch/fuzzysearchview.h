/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QScrollArea>
#include <QDropEvent>
#include <QList>

// Local includes

#include "statesavingobject.h"
#include "album.h"

class QMimeData;
class QPixmap;

namespace Digikam
{

class FuzzySearchFolderView;
class ItemInfo;
class LoadingDescription;
class SearchModel;
class SearchModificationHelper;
class SearchTextBar;

class FuzzySearchView : public QScrollArea,
                        public StateSavingObject
{
    Q_OBJECT

public:

    explicit FuzzySearchView(SearchModel* const searchModel,
                             SearchModificationHelper* const searchModificationHelper,
                             QWidget* const parent = nullptr);
    ~FuzzySearchView() override;

    SAlbum* currentAlbum() const;
    void setCurrentAlbum(SAlbum* const album);

    void setActive(bool val);
    void setItemInfo(const ItemInfo& info);

    void newDuplicatesSearch(const QList<PAlbum*>& albums);
    void newDuplicatesSearch(const QList<TAlbum*>& albums);

    void setConfigGroup(const KConfigGroup& group) override;
    void doLoadState()                                     override;
    void doSaveState()                                     override;

protected:

    void dragEnterEvent(QDragEnterEvent* e)                override;
    void dragMoveEvent(QDragMoveEvent* e)                  override;
    void dropEvent(QDropEvent* e)                          override;

Q_SIGNALS:

    void signalNotificationError(const QString& message, int type);

private Q_SLOTS:

    void slotTabChanged(int);

    void slotHSChanged(int h, int s);
    void slotVChanged(int v);
    void slotPenColorChanged(const QColor&);
    void slotClearSketch();
    void slotSaveSketchSAlbum();
    void slotCheckNameEditSketchConditions();

    void slotAlbumSelected(Album* album);

    void slotSaveImageSAlbum();
    void slotCheckNameEditImageConditions();
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

    void slotDirtySketch();
    void slotTimerSketchDone();
    void slotUndoRedoStateChanged(bool, bool);

    void slotMinLevelImageChanged(int);
    void slotMaxLevelImageChanged(int);
    void slotFuzzyAlbumsChanged();
    void slotTimerImageDone();

    void slotApplicationSettingsChanged();

private:

    void setCurrentImage(qlonglong imageid);
    void setCurrentImage(const ItemInfo& info);

    void createNewFuzzySearchAlbumFromSketch(const QString& name, bool force = false);
    void createNewFuzzySearchAlbumFromImage(const QString& name, bool force = false);
    bool dragEventWrapper(const QMimeData* data) const;

    void setColor(QColor c);

    QWidget* setupFindSimilarPanel()             const;
    QWidget* setupSketchPanel()                  const;
    void     setupConnections();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
