/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : Item delegate for import interface items.
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "thumbnailsize.h"
#include "ditemdelegate.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class ImportCategoryDrawer;
class ImportCategorizedView;
class ImportFilterModel;
class ImportItemModel;
class ItemViewImportDelegatePrivate;

// NOTE: Some reuse of the existing model-view classes.

class ItemViewImportDelegate : public DItemDelegate,
                               public ItemDelegateOverlayContainer
{
    Q_OBJECT

public:

    explicit ItemViewImportDelegate(QWidget* const parent);
    ~ItemViewImportDelegate() override;

public:

    ThumbnailSize thumbnailSize()                                                     const;
    double        displayRatio()                                                      const;
    int           spacing()                                                           const;
    QRect         rect()                                                              const;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index)      const override;
    QSize gridSize()                                                                  const override;

    /// reimplemented from DItemDelegate

    void setThumbnailSize(const ThumbnailSize& thumbSize)                                   override;
    void setSpacing(int _spacing)                                                           override;
    void setDefaultViewOptions(const QStyleOptionViewItem& option)                          override;
    bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                        const QModelIndex& index, QRect* tooltipRect = nullptr)       const override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                           const QModelIndex& index, QRect* activationRect = nullptr) const override;

    /**
     * Returns the area where the pixmap is drawn,
     * or null if not supported
     */
    virtual QRect pixmapRect()                                                        const;

    /**
     * Returns the area where the image information is drawn,
     * or null if empty / not supported.
     * The image information is textual or graphical information,
     * but not the pixmap. The ratingRect() will e.g. typically
     * be contained in this area.
     */
    virtual QRect imageInformationRect()                                              const;

    /**
     * Can be used to temporarily disable drawing of the rating.
     * Call with QModelIndex() afterwards.
     */
    void setRatingEdited(const QModelIndex& index);

    /**
     * Returns the rectangle where the rating is drawn,
     * or a null rectangle if not supported.
     */
    virtual QRect ratingRect() const;

    void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)      override;

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

    void overlayDestroyed(QObject* o)                                                       override;

Q_SIGNALS:

    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected:

    /// Use the tool methods for painting in subclasses
    QRect drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background, const QPixmap& thumbnail) const;
    void drawRating(QPainter* _painter, const QModelIndex& _index,
                    const QRect& _ratingRect, int _rating, bool _isSelected)                                      const;
    void drawName(QPainter* p,const QRect& nameRect, const QString& name)                                         const;
    void drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)                              const;
    void drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims)                                     const;
    void drawImageFormat(QPainter* p, const QRect& dimsRect, const QString& mime)                                 const;
    void drawFileSize(QPainter* p, const QRect& r, qlonglong bytes)                                               const;
    void drawTags(QPainter* p, const QRect& r, const QString& tagsString, bool isSelected)                        const;
    void drawGroupIndicator(QPainter* p, const QRect& r, int numberOfGroupedImages, bool open)                    const;
    void drawPickLabelIcon(QPainter* p, const QRect& r, int pickLabel)                                            const;
    void drawColorLabelLine(QPainter* p, const QRect& pixRect, int colorId)                                       const;
    void drawGeolocationIndicator(QPainter* p, const QRect& r)                                                    const;
    void drawDownloadIndicator(QPainter* p, const QRect& r, int itemType)                                         const;
    void drawLockIndicator(QPainter* p, const QRect& r, int lockStatus)                                           const;
    void drawFocusRect(QPainter* p, const QStyleOptionViewItem& option, bool isSelected)                          const;
    void drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option)                                       const;
    void prepareFonts();
    void prepareMetrics(int maxWidth);
    void prepareBackground();
    void prepareRatingPixmaps(bool composeOverBackground = true);

    /// Returns the relevant pixmap from the cached rating pixmaps
    QPixmap ratingPixmap(int rating, bool selected)                                                               const;

    QAbstractItemDelegate* asDelegate()                                                                                 override;

    /// reimplement these in subclasses
    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps() = 0;

protected:

    ItemViewImportDelegatePrivate* const d_ptr = nullptr;
    ItemViewImportDelegate(ItemViewImportDelegatePrivate& dd, QWidget* const parent);

private:

    /// @note disabled
    ItemViewImportDelegate(const ItemViewImportDelegate&)            = delete;
    ItemViewImportDelegate& operator=(const ItemViewImportDelegate&) = delete;

private:

    Q_DECLARE_PRIVATE(ItemViewImportDelegate)
};

} // namespace Digikam
