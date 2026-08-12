/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Matthias Kretz <kretz at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dconfigdlgmodels.h"

namespace Digikam
{

class Q_DECL_HIDDEN DConfigDlgModelPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgModel)

public:

    DConfigDlgModelPrivate()          = default;
    virtual ~DConfigDlgModelPrivate() = default;

protected:

    DConfigDlgModel* q_ptr = nullptr;

private:

    Q_DISABLE_COPY(DConfigDlgModelPrivate)
};

// ----------------------------------------------------------------------------

class Q_DECL_HIDDEN PageItem
{
public:

    explicit PageItem(DConfigDlgWdgItem* const pageItem, PageItem* const parent = nullptr);
    ~PageItem();

    void appendChild(PageItem* const child);
    void insertChild(int row, PageItem* const child);
    void removeChild(int row);

    PageItem* child(int row);
    int childCount()                    const;
    int columnCount()                   const;
    int row()                           const;
    PageItem* parent()                  const;

    DConfigDlgWdgItem* pageWidgetItem() const;

    PageItem* findChild(const DConfigDlgWdgItem* item);

    void dump(int indent = 0);

private:

    DConfigDlgWdgItem* mPageWidgetItem = nullptr;
    QList<PageItem*>   mChildItems;
    PageItem*          mParentItem     = nullptr;

private:

    /// @note disabled
    PageItem(const PageItem&)            = delete;
    PageItem& operator=(const PageItem&) = delete;
};

// ----------------------------------------------------------------------------

class Q_DECL_HIDDEN DConfigDlgWdgModelPrivate : public DConfigDlgModelPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgWdgModel)

protected:

    DConfigDlgWdgModelPrivate()
        : rootItem(new PageItem(nullptr, nullptr))
    {
    }

    ~DConfigDlgWdgModelPrivate() override
    {
        delete rootItem;
        rootItem = nullptr;
    }

    void _k_itemChanged()
    {
        Q_Q(DConfigDlgWdgModel);

        const DConfigDlgWdgItem* const item = qobject_cast<DConfigDlgWdgItem*>(q->sender());

        if (!item)
        {
            return;
        }

        const QModelIndex index = q->index(item);

        if (!index.isValid())
        {
            return;
        }

        Q_EMIT q->dataChanged(index, index);
    }

    void _k_itemToggled(bool checked)
    {
        Q_Q(DConfigDlgWdgModel);

        DConfigDlgWdgItem* const item = qobject_cast<DConfigDlgWdgItem*>(q->sender());

        if (!item)
        {
            return;
        }

        Q_EMIT q->toggled(item, checked);
    }

protected:

    PageItem* rootItem = nullptr;

private:

    /// @note disabled
    DConfigDlgWdgModelPrivate(const DConfigDlgWdgModelPrivate&)            = delete;
    DConfigDlgWdgModelPrivate& operator=(const DConfigDlgWdgModelPrivate&) = delete;
};

} // namespace Digikam
