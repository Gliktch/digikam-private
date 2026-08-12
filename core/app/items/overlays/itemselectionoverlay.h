/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : selection icon view item at mouse hover
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

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class ItemSelectionOverlayButton : public ItemViewHoverButton
{
    Q_OBJECT

public:

    explicit ItemSelectionOverlayButton(QAbstractItemView* const parentView);
    QSize sizeHint() const  override;

protected:

    QIcon icon()            override;
    void updateToolTip()    override;
};

// --------------------------------------------------------------------

class ItemSelectionOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ItemSelectionOverlay(QObject* const parent);
    void setActive(bool active)                 override;

protected:

    ItemViewHoverButton* createButton()         override;
    void updateButton(const QModelIndex& index) override;

protected Q_SLOTS:

    void slotClicked(bool checked);
    void slotSelectionChanged(const QItemSelection&, const QItemSelection&);
};

} // namespace Digikam
