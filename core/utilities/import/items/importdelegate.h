/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view to import items - the delegate
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include <QtGlobal>
#include <QListView>

// Local includes

#include "itemviewimportdelegate.h"
#include "importthumbnailmodel.h"
#include "importcategorydrawer.h"

namespace Digikam
{

class ImportCategorizedView;
class ImportThumbnailDelegatePrivate;
class ImportNormalDelegatePrivate;

class ImportDelegate : public ItemViewImportDelegate
{
    Q_OBJECT

public:

    explicit ImportDelegate(QWidget* const parent);
    ~ImportDelegate()                                                                                  override;

    void setView(ImportCategorizedView* view);

    ImportCategoryDrawer* categoryDrawer()                                                       const;

/*
    QRect commentsRect()                                                                         const;
*/
    QRect tagsRect()                                                                             const;
    QRect actualPixmapRect(const QModelIndex& index)                                             const;
    QRect groupIndicatorRect()                                                                   const;
    QRect downloadIndicatorRect()                                                                const;
    QRect lockIndicatorRect()                                                                    const;
    QRect coordinatesIndicatorRect()                                                             const;

    int calculatethumbSizeToFit(int ws);

    void setSpacing(int spacing)                                                                       override;
    void setDefaultViewOptions(const QStyleOptionViewItem& option)                                     override;
    bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                                const QModelIndex& index, QRect* tooltipRect = nullptr)          const override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                   const QModelIndex& index, QRect* activationRect = nullptr)    const override;

    QRect pixmapRect()                                                                           const override;
    QRect imageInformationRect()                                                                 const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)  const override;
    QPixmap pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const override;

    /**
     * Retrieve the thumbnail pixmap in given size for the ImportItemModel::ThumbnailRole for
     * the given index from the given index, which must adhere to ImportThumbnailModel semantics.
     */
    static QPixmap retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize);

public:

    // Declared as public because of use in ImportNormalDelegate class.
    class ImportDelegatePrivate;

protected:

    bool onActualPixmapRect(const QPoint& pos, const QRect& visualRect,
                            const QModelIndex& index, QRect* actualRect) const;
    void updateActualPixmapRect(const QModelIndex& index, const QRect& rect);

    void setModel(QAbstractItemModel* model);

    ImportDelegate(ImportDelegate::ImportDelegatePrivate& dd, QWidget* const parent);

    /**
     * Reimplement this to set contentWidth. This is the maximum width of all
     * content rectangles, typically excluding margins on both sides.
     */
    virtual void updateContentWidth();

    /**
     * In a subclass, you need to implement this method to set up the rects
     * for drawing. The paint() method operates depending on these rects.
     */
    virtual void updateRects() = 0;

    void clearCaches()               override;

    /**
     * Reimplement to clear caches based on model indexes (hash on row number etc.)
     * Change signals are listened to this is called whenever such properties become invalid.
     */
    virtual void clearModelDataCaches();

    virtual QPixmap thumbnailPixmap(const QModelIndex& index) const;

    void invalidatePaintingCache()   override;
    void updateSizeRectsAndPixmaps() override;

protected Q_SLOTS:

    void modelChanged();
    void modelContentsChanged();

private:

    Q_DECLARE_PRIVATE(ImportDelegate)
};

// ------ ImportThumbnailDelegate ----------------------------------------

class ImportThumbnailDelegate : public ImportDelegate
{
    Q_OBJECT

public:

    explicit ImportThumbnailDelegate(ImportCategorizedView* const parent);
    ~ImportThumbnailDelegate() override;

    void setFlow(QListView::Flow flow);

    /**
     * Returns the minimum or maximum viewport size in the limiting dimension,
     * width or height, depending on current flow.
     */
    int maximumSize() const;
    int minimumSize() const;

    void setDefaultViewOptions(const QStyleOptionViewItem& option) override;
    bool acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                           QRect* activationRect) const            override;

protected:

    void updateContentWidth()                                      override;
    void updateRects()                                             override;
    int thumbnailPixmapSize(bool withHighlight, int size);

private:

    Q_DECLARE_PRIVATE(ImportThumbnailDelegate)
};

// ------ ImportNormalDelegate ----------------------------------------

class ImportNormalDelegate : public ImportDelegate
{
    Q_OBJECT

public:

    explicit ImportNormalDelegate(ImportCategorizedView* const parent);
    ~ImportNormalDelegate() override;

protected:

    ImportNormalDelegate(ImportNormalDelegatePrivate& dd, ImportCategorizedView* const parent);

    void updateRects()      override;

private:

    Q_DECLARE_PRIVATE(ImportNormalDelegate)
};

} // namespace Digikam
