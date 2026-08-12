/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rejection icon view item on mouse hover
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
#include "iteminfo.h"

namespace Digikam
{

class FaceRejectionOverlayButton : public ItemViewHoverButton
{
    Q_OBJECT

public:

    explicit FaceRejectionOverlayButton(QAbstractItemView* const parentView);
    QSize sizeHint() const  override;

protected:

    QIcon icon()            override;
    void updateToolTip()    override;
};

// --------------------------------------------------------------------

class FaceRejectionOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit FaceRejectionOverlay(QObject* const parent);
    void setActive(bool active)                     override;

Q_SIGNALS:

    void rejectFaces(const QList<QModelIndex>& indexes);

protected:

    ItemViewHoverButton* createButton()             override;
    void updateButton(const QModelIndex& index)     override;
    bool checkIndex(const QModelIndex& index) const override;
    void widgetEnterEvent()                         override;
    void widgetLeaveEvent()                         override;

protected Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam
