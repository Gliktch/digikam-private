/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Managing of face tag region items on a GraphicsDImgView
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPointF>

class QGraphicsSceneHoverEvent;

namespace Digikam
{

class Album;
class ItemInfo;
class ImageTagChangeset;
class GraphicsDImgView;
class RegionFrameItem;
class TaggingAction;

class FaceGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:

    /**
     * Constructs a new face group, managing RegionFrameItems for faces
     * of a particular image, displayed on a GraphicsDImgView.
     */
    explicit FaceGroup(GraphicsDImgView* const view);
    ~FaceGroup() override;

    bool isVisible()                                                     const;
    ItemInfo info()                                                      const;
    bool hasVisibleItems()                                               const;

    QList<RegionFrameItem*> items()                                      const;

    /**
     * Auto suggest applies if an image has not been scanned,
     * or an unknown face is registered.
     * In this case, a new scan will be triggered.
     */
    void setAutoSuggest(bool doAutoSuggest);
    bool autoSuggest()                                                   const;

    /**
     * Even if visible() is false, show the item under the mouse cursor.
     */
    void setShowOnHover(bool show);
    bool showOnHover()                                                   const;

    /**
     * Returns the item in this group closest to scene position p.
     * If given, manhattanLength is set to the manhattan length between
     * p and the closest point of the returned item's bounding rectangle.
     * In particular, if p is inside the item's rectangle, manhattanLength is 0.
     */
    RegionFrameItem* closestItem(const QPointF& p,
                                 qreal* const manhattanLength = nullptr) const;

    bool acceptsMouseClick(const QPointF& scenePos);
    void itemHoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void itemHoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void itemHoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    void leaveEvent(QEvent*);
    void enterEvent(QEvent*);

    /**
     * Returns a boolean whether there is at least one unconfirmed face in the group or not.
     */
    bool hasUnconfirmed();

public Q_SLOTS:

    /**
     * Shows or hides the frames.
     */
    void setVisible(bool visible);
    void setVisibleItem(RegionFrameItem* item);

    /**
     * Sets the current ItemInfo.
     */
    void setInfo(const ItemInfo& info);

    /**
     * Prepares to load a new info.
     * Closes the face group for editing.
     * Pass a null info if about to close.
     */
    void aboutToSetInfo(const ItemInfo& info);

    /**
     * Enters a special state where by click + drag a new face can be created.
     */
    void addFace();

    /**
     * Rejects (clears) all faces on the current image.
     */
    void rejectAll();

    /**
     * Mark all unconfirmed faces as ignored on the current image.
     */
    void markAllAsIgnored();

protected:

    void load();
    void applyItemGeometryChanges();

protected Q_SLOTS:

    void itemStateChanged(int);
    void startAutoSuggest();
    void slotAlbumsUpdated(int type);
    void slotAlbumRenamed(Album* album);
    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotAssigned(const TaggingAction& action, const ItemInfo& info, const QVariant& faceIdentifier);
    void slotRejected(const ItemInfo& info, const QVariant& faceIdentifier);
    void slotIgnored(const ItemInfo& info, const QVariant& faceIdentifier);
    void slotLabelClicked(const ItemInfo& info, const QVariant& faceIdentifier);
    void slotIgnoredClicked(const ItemInfo& info, const QVariant& faceIdentifier);
    void slotAddItemStarted(const QPointF& pos);
    void slotAddItemMoving(const QRectF& rect);
    void slotAddItemFinished(const QRectF& rect);
    void slotCancelAddItem();
    void slotFocusRandomFace();

private:

    /// @note disabled
    FaceGroup()                            = delete;
    FaceGroup(const FaceGroup&)            = delete;
    FaceGroup& operator=(const FaceGroup&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
