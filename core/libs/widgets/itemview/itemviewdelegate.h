/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for items - the delegate
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ditemdelegate.h"
#include "itemdelegateoverlay.h"
#include "thumbnailsize.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemViewDelegatePrivate;

class DIGIKAM_EXPORT ItemViewDelegate : public DItemDelegate,
                                        public ItemDelegateOverlayContainer
{
    Q_OBJECT

public:

    explicit ItemViewDelegate(QWidget* const parent);
    ~ItemViewDelegate() override;

    ThumbnailSize thumbnailSize()                                                             const;
    double        displayRatio()                                                              const;
    int           spacing()                                                                   const;
    QRect         rect()                                                                      const;

    /**
     * Can be used to temporarily disable drawing of the rating.
     * Call with QModelIndex() afterwards.
     */
    void setRatingEdited(const QModelIndex& index);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index)              const override;
    QSize gridSize()                                                                          const override;

    // reimplemented from DItemDelegate
    void setThumbnailSize(const ThumbnailSize& thumbSize)                                           override;
    void setSpacing(int spacing)                                                                    override;
    void setDefaultViewOptions(const QStyleOptionViewItem& option)                                  override;
    bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                                const QModelIndex& index, QRect* tooltipRect = nullptr)       const override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                   const QModelIndex& index, QRect* activationRect = nullptr) const override;

    /**
     * Returns the area where the pixmap is drawn,
     * or null if not supported.
     */
    virtual QRect pixmapRect()                                                                const;

    /**
     * Returns the area where the image information is drawn,
     * or null if empty / not supported.
     * The image information is textual or graphical information,
     * but not the pixmap. The ratingRect() will e.g. typically
     * be contained in this area.
     */
    virtual QRect imageInformationRect()                                                      const;

    /**
     * Returns the rectangle where the rating is drawn,
     * or a null rectangle if not supported.
     */
    virtual QRect ratingRect()                                                                const;

    void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)              override;

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

    void overlayDestroyed(QObject* o)                                                               override;

Q_SIGNALS:

    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected:

    ItemViewDelegate(ItemViewDelegatePrivate& dd, QWidget* const parent);

    /**
     * Use the tool methods for painting in subclasses
     */
    QRect drawThumbnail(QPainter* p, const QRect& thumbRect,
                        const QPixmap& background, const QPixmap& thumbnail, bool isGrouped)  const;
    void drawRating(QPainter* p, const QModelIndex& index,
                    const QRect& ratingRect, int rating, bool isSelected)                     const;
    void drawSpecialInfo(QPainter* p,const QRect& r, const QString& text)                     const;
    void drawName(QPainter* p,const QRect& nameRect, const QString& name)                     const;
    void drawTitle(QPainter *p, const QRect& titleRect, const QString& title)                 const;
    void drawComments(QPainter* p, const QRect& commentsRect, const QString& comments)        const;
    void drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)          const;
    void drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)      const;
    void drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims)                 const;
    void drawAspectRatio(QPainter* p, const QRect& dimsRect, const QSize& dims)               const;
    void drawFileSize(QPainter* p, const QRect& r, qlonglong bytes)                           const;
    void drawTags(QPainter* p, const QRect& r, const QString& tagsString, bool isSelected)    const;
    void drawImageFormat(QPainter* p, const QRect& r, const QString& f, bool drawTop)         const;
    void drawColorLabelLine(QPainter* p, const QRect& pixRect, int colorId)                   const;
    void drawPickLabelIcon(QPainter* p, const QRect& r, int pickLabel)                        const;
    void drawGroupIndicator(QPainter* p, const QRect& r,
                            int numberOfGroupedImages, bool open)                             const;
    void drawGeolocationIndicator(QPainter* p, const QRect& r)                                const;
    void drawPanelSideIcon(QPainter* p, bool left, bool right)                                const;
    void drawFocusRect(QPainter* p, const QStyleOptionViewItem& option, bool isSelected)      const;
    void drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option)                   const;
    void drawGroupedRect(QPainter* p, const QStyleOptionViewItem& option)                     const;
    void prepareFonts();
    void prepareMetrics(int maxWidth);
    void prepareBackground();
    void prepareRatingPixmaps(bool composeOverBackground = true);

    /**
     * Returns the relevant pixmap from the cached rating pixmaps.
     */
    QPixmap ratingPixmap(int rating, bool selected)                                           const;

    QAbstractItemDelegate* asDelegate()                                                             override;

    // reimplement these in subclasses
    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps() = 0;

protected:

    ItemViewDelegatePrivate* const d_ptr = nullptr;

private:

    /// @note disabled
    ItemViewDelegate(const ItemViewDelegate&);
    ItemViewDelegate& operator=(const ItemViewDelegate&);

    Q_DECLARE_PRIVATE(ItemViewDelegate)
};

} // namespace Digikam
