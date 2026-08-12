/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : A DItemToolTip prepared for use in QAbstractItemViews
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "ditemtooltip.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ItemViewToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit ItemViewToolTip(QAbstractItemView* const view);
    ~ItemViewToolTip()                      override;

    virtual QAbstractItemView* view() const;

    QAbstractItemModel* model()       const;
    QModelIndex currentIndex()        const;

    /**
     * Show the tooltip for the given item.
     * The rect of the given option is taken as area for which
     * the tooltip is shown.
     */
    void show(const QStyleOptionViewItem& option, const QModelIndex& index);

    void setTipContents(const QString& tipContents);

    /**
     * Default implementation is based on setTipContents().
     * Reimplement if you dynamically provide the contents.
     */
    QString tipContents()                   override;

protected:

    QRect repositionRect()                  override;

    bool eventFilter(QObject* o, QEvent* e) override;
    void hideEvent(QHideEvent*)             override;
    void mouseMoveEvent(QMouseEvent* e)     override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
