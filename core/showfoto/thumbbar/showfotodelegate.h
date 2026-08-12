/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt model view for Showfoto item - the delegate
 *
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QListView>

// Local includes

#include "showfotoitemviewdelegate.h"
#include "showfotothumbnailmodel.h"

namespace ShowFoto
{

class ShowfotoThumbnailBar;
class ShowfotoThumbnailDelegatePrivate;
class ShowfotoNormalDelegatePrivate;
class ShowfotoFilterModel;

class ShowfotoDelegate : public ShowfotoItemViewDelegate
{
    Q_OBJECT

public:

    explicit ShowfotoDelegate(QWidget* const parent);
    ~ShowfotoDelegate()                                                                                override;

    void setView(ShowfotoThumbnailBar* view);

    QRect actualPixmapRect(const QModelIndex& index)                                             const;
    QRect groupIndicatorRect()                                                                   const;
    QRect coordinatesIndicatorRect()                                                             const;

    int calculatethumbSizeToFit(int ws);

    bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                        const QModelIndex& index, QRect* tooltipRect = nullptr)                  const override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                           const QModelIndex& index, QRect* activationRect = nullptr)            const override;

    QRect pixmapRect()                                                                           const override;
    QRect imageInformationRect()                                                                 const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)  const override;
    QPixmap pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const override;

    /**
     * @brief retrieve the thumbnail pixmap in given size for the ShowfotoItemModel::ThumbnailRole for
     * the given index from the given index, which must adhere to ShowfotoThumbnailModel semantics.
     */
    static QPixmap retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize);

public:

    /// @note declared as public because of use in ShowfotoNormalDelegate class.
    class ShowfotoDelegatePrivate;

protected:

    bool onActualPixmapRect(const QPoint& pos, const QRect& visualRect,
                            const QModelIndex& index, QRect* actualRect)                         const;
    void updateActualPixmapRect(const QModelIndex& index, const QRect& rect);

    void setModel(QAbstractItemModel* model);

    ShowfotoDelegate(ShowfotoDelegate::ShowfotoDelegatePrivate& dd, QWidget* const parent);

    /**
     * @brief reimplement this to set contentWidth. This is the maximum width of all
     * content rectangles, typically excluding margins on both sides.
     */
    virtual void updateContentWidth();

    /**
     * @brief in a subclass, you need to implement this method to set up the rects
     * for drawing. The paint() method operates depending on these rects.
     */
    virtual void updateRects() = 0;

    void clearCaches()                                                                                 override;

    /**
     * @brief reimplement to clear caches based on model indexes (hash on row number etc.)
     * Change signals are listened to this is called whenever such properties become invalid.
     */
    virtual void clearModelDataCaches();

    virtual QPixmap thumbnailPixmap(const QModelIndex& index)                                    const;

    void updateSizeRectsAndPixmaps()                                                                   override;

protected Q_SLOTS:

    void modelChanged();
    void modelContentsChanged();

private:

    Q_DECLARE_PRIVATE(ShowfotoDelegate)
};

// -------------- ShowfotoThumbnailDelegate ---------------------

class ShowfotoThumbnailDelegate : public ShowfotoDelegate
{
    Q_OBJECT

public:

    explicit ShowfotoThumbnailDelegate(ShowfotoThumbnailBar* const parent);
    ~ShowfotoThumbnailDelegate()                                   override;

    void setFlow(QListView::Flow flow);

    /**
     * @return the minimum or maximum viewport size in the limiting dimension,
     * width or height, depending on current flow.
     */
    int maximumSize()                                        const;
    int minimumSize()                                        const;

    void setDefaultViewOptions(const QStyleOptionViewItem& option) override;
    bool acceptsActivation(const QPoint& pos,
                           const QRect& visualRect,
                           const QModelIndex& index,
                           QRect* activationRect)            const override;

protected:

    void updateContentWidth()                                      override;
    void updateRects()                                             override;
    int thumbnailPixmapSize(bool withHighlight, int size);

private:

    /// @note disabled
    explicit ShowfotoThumbnailDelegate(QObject*) = delete;

private:

    Q_DECLARE_PRIVATE(ShowfotoThumbnailDelegate)
};

// ------------------- ShowfotoNormalDelegate ------------------

class ShowfotoNormalDelegate : public ShowfotoDelegate
{
    Q_OBJECT

public:

    explicit ShowfotoNormalDelegate(ShowfotoThumbnailBar* const bar,
                                    QObject* const parent = nullptr);
    ~ShowfotoNormalDelegate()   override;

protected:

    ShowfotoNormalDelegate(ShowfotoNormalDelegatePrivate& dd,
                           ShowfotoThumbnailBar* const bar,
                           QObject* const parent = nullptr);

    void updateRects()          override;

private:

    Q_DECLARE_PRIVATE(ShowfotoNormalDelegate)
};

} // namespace ShowFoto
