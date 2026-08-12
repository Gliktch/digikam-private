/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : Qt model-view for items - category drawer
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dcategorydrawer.h"

class QStyleOptionViewItem;

namespace Digikam
{

class ItemCategorizedView;
class Album;
class PAlbum;
class TAlbum;
class SAlbum;
class DAlbum;

class ItemCategoryDrawer : public DCategoryDrawer
{
    Q_OBJECT

public:

    explicit ItemCategoryDrawer(ItemCategorizedView* const parent);
    ~ItemCategoryDrawer()                                                                                                  override;

    int categoryHeight(const QModelIndex& index, const QStyleOption& option)                                         const override;
    void drawCategory(const QModelIndex& index, int sortRole, const QStyleOption& option, QPainter* painter)         const override;
    virtual int maximumHeight()                                                                                      const;

    void setLowerSpacing(int spacing);
    void setDefaultViewOptions(const QStyleOptionViewItem& option);
    void invalidatePaintingCache();

private:

    void updateRectsAndPixmaps(int width);
    void viewHeaderText(const QModelIndex& index, QString* header, QString* subLine)                                 const;
    void textForAlbum(const QModelIndex& index, QString* header, QString* subLine)                                   const;
    void textForPAlbum(PAlbum* a, bool recursive, int count, QString* header, QString* subLine)                      const;
    void textForTAlbum(TAlbum* a, bool recursive, int count, QString* header, QString* subLine)                      const;
    void textForSAlbum(SAlbum* a, int count, QString* header, QString* subLine)                                      const;
    void textForDAlbum(DAlbum* a, int count, QString* header, QString* subLine)                                      const;
    void textForFormat(const QModelIndex& index, QString* header, QString* subLine)                                  const;
    void textForFace(const QModelIndex& index, QString* header, QString* subLine)                                    const;
    void textForMonth(const QModelIndex& index, QString* header, QString* subLine)                                   const;
    void textForDay(const QModelIndex& index, QString* header, QString* subLine)                                     const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
