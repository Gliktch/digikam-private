/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt model view for Showfoto item - the delegate
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "thumbnailsize.h"
#include "ditemdelegate.h"
#include "itemdelegateoverlay.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoFilterModel;
class ShowfotoItemModel;
class ShowfotoItemViewDelegatePrivate;

class ShowfotoItemViewDelegate : public DItemDelegate,
                                 public ItemDelegateOverlayContainer
{
    Q_OBJECT

public:

    explicit ShowfotoItemViewDelegate(QWidget* const parent);
    ~ShowfotoItemViewDelegate()                                                               override;

    ThumbnailSize thumbnailSize()                                                       const;
    double        displayRatio()                                                        const;
    int           spacing()                                                             const;
    QRect         rect()                                                                const;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index)                                            const override;
    QSize gridSize()                                                                    const override;

    /// reimplemented from DItemDelegate
    void setThumbnailSize(const ThumbnailSize& thumbSize)                                     override;
    void setSpacing(int spacing)                                                              override;
    void setDefaultViewOptions(const QStyleOptionViewItem& option)                            override;
    bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                        const QModelIndex& index, QRect* tooltipRect = nullptr)         const override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                           const QModelIndex& index, QRect* activationRect = nullptr)   const override;

    /**
     * Returns the area where the pixmap is drawn,
     * or null if not supported
     */
    virtual QRect pixmapRect()                                                          const;

    /**
     * Returns the area where the image information is drawn,
     * or null if empty / not supported.
     * The image information is textual or graphical information,
     * but not the pixmap. The ratingRect() will e.g. typically
     * be contained in this area.
     */
    virtual QRect imageInformationRect()                                                const;

    void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)        override;

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

    void overlayDestroyed(QObject* o)                                                         override;

Q_SIGNALS:

    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected:

    /// Use the tool methods for painting in subclasses
    QRect drawThumbnail(QPainter* p, const QRect& thumbRect,
                        const QPixmap& background, const QPixmap& thumbnail)            const;
    void drawName(QPainter* p,const QRect& nameRect, const QString& name)               const;
    void drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)    const;
    void drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims)           const;
    void drawImageFormat(QPainter* p, const QRect& dimsRect, const QString& mime)       const;
    void drawFileSize(QPainter* p, const QRect& r, qlonglong bytes)                     const;
    void drawGeolocationIndicator(QPainter* p, const QRect& r)                          const;
    void drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                       bool isSelected)                                                 const;
    void drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option)             const;
    void prepareFonts();
    void prepareMetrics(int maxWidth);
    void prepareBackground();

    QAbstractItemDelegate* asDelegate()                                                       override;

    /// reimplement these in subclasses
    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps()                                            = 0;

protected:

    ShowfotoItemViewDelegatePrivate* const d_ptr = nullptr;

protected:

    ShowfotoItemViewDelegate(ShowfotoItemViewDelegatePrivate& dd, QWidget* const parent);

private:

    /// @note disabled.
    ShowfotoItemViewDelegate(const ShowfotoItemViewDelegate&)            = delete;
    ShowfotoItemViewDelegate& operator=(const ShowfotoItemViewDelegate&) = delete;

private:

    Q_DECLARE_PRIVATE(ShowfotoItemViewDelegate)
};

} // namespace ShowFoto
