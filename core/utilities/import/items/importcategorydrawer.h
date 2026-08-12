/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-17
 * Description : Qt item view for images - category drawer
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStyleOptionViewItem>

// Local includes

#include "dcategorydrawer.h"

namespace Digikam
{

class ImportCategorizedView;

class ImportCategoryDrawer : public DCategoryDrawer
{
    Q_OBJECT

public:

    explicit ImportCategoryDrawer(ImportCategorizedView* const parent);
    ~ImportCategoryDrawer()                                                                   override;

    void setLowerSpacing(int spacing);
    void setDefaultViewOptions(const QStyleOptionViewItem& option);
    void invalidatePaintingCache();

    int  categoryHeight(const QModelIndex& index, const QStyleOption& option)           const override;
    void drawCategory(const QModelIndex& index, int sortRole,
                      const QStyleOption& option, QPainter* painter)                    const override;
    virtual int  maximumHeight()                                                        const;

private:

    void updateRectsAndPixmaps(int width);
    void viewHeaderText(const QModelIndex& index, QString* header, QString* subLine)    const;
    void textForFormat(const QModelIndex& index, QString* header, QString* subLine)     const;
    void textForDate(const QModelIndex& index, QString* header, QString* subLine)       const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
