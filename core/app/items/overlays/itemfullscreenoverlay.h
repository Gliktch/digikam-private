/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-30
 * Description : fullscreen overlay
 *
 * SPDX-FileCopyrightText:      2015 by Luca Carlon <carlon dot luca at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class ItemFullScreenOverlayButton : public ItemViewHoverButton
{
    Q_OBJECT

public:

    explicit ItemFullScreenOverlayButton(QAbstractItemView* const parentView);
    QSize sizeHint() const  override;

protected:

    QIcon icon()            override;
    void updateToolTip()    override;
};

// --------------------------------------------------------------------

class ItemFullScreenOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ItemFullScreenOverlay(QObject* const parent);
    void setActive(bool active)                     override;

    static ItemFullScreenOverlay* instance(QObject* const parent);

Q_SIGNALS:

    void signalFullscreen(const QList<QModelIndex>& indexes);

protected:

    ItemViewHoverButton* createButton()             override;
    void updateButton(const QModelIndex& index)     override;
    bool checkIndex(const QModelIndex& index) const override;
    void widgetEnterEvent()                         override;
    void widgetLeaveEvent()                         override;

private Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam
