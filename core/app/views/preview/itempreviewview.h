/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to play video preview.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAction>
#include <QIODevice>
#include <QMap>
#include <QSharedPointer>
#include <QString>

// Local includes

#include "digikam_globals.h"
#include "graphicsdimgview.h"
#include "iteminfo.h"

namespace Digikam
{

class Album;
class ImageChangeset;
class ImageTagChangeset;
class LoadingDescription;

class ItemPreviewView : public GraphicsDImgView
{
    Q_OBJECT

public:

    enum Mode
    {
        IconViewPreview,
        LightTablePreview
    };

public:

    explicit ItemPreviewView(QWidget* const parent,
                             Mode mode = IconViewPreview,
                             Album* const currAlbum = nullptr);
    ~ItemPreviewView() override;

    void setItemInfo(const ItemInfo& info     = ItemInfo(),
                     const ItemInfo& previous = ItemInfo(),
                     const ItemInfo& next     = ItemInfo());

    ItemInfo getItemInfo() const;

    void reload();
    void setImagePath(const QString& path = QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);
    void setHostWindowActions(const HostActionsMap& actions);

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalPreviewLoaded(bool success);
    void signalEscapePreview();
    void signalAddToExistingQueue(int);
    void signalSlideShowCurrent();
    void signalPlayMotionPhoto(const QSharedPointer<QIODevice>& videoData, const QUrl& sourceUrl, int orientation);

    void signalGotoAlbumAndItem(const ItemInfo&);
    void signalGotoDateAndItem(const ItemInfo&);
    void signalGotoTagAndItem(int);
    void signalPopupTagsView();

    void signalStartedLoading();
    void signalLoadingProgress(float progress);
    void signalLoadingComplete();

protected:

    bool acceptsMouseClick(QMouseEvent* e)  override;
    void mousePressEvent(QMouseEvent* e)    override;
    void leaveEvent(QEvent* e)              override;
    void showEvent(QShowEvent* e)           override;
    void dropEvent(QDropEvent* e)           override;
    void dragMoveEvent(QDragMoveEvent* e)   override;
    void dragEnterEvent(QDragEnterEvent* e) override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent*)           override;

#else

    void enterEvent(QEvent*)                override;

#endif

private:

    void setupOverlays();

private Q_SLOTS:

    void slotItemLoaded();
    void slotItemLoadingFailed();
    void slotPlayMotionPhoto();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);

    void slotThemeChanged();
    void slotSetupChanged();

    void slotRotateLeft();
    void slotRotateRight();
    void slotDeleteItem();

    /**
     * @brief slotUpdateFaces - after applying some transformation on
     *                          image, update face tags position
     */
    void slotUpdateFaces();

    void slotImageChange(const ImageChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);

    void slotShowContextMenu(QGraphicsSceneContextMenuEvent* event);

    void slotSlideShowCurrent();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
