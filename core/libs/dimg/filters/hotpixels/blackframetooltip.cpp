/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-20
 * Description : black frame list view tool tip
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blackframetooltip.h"

// Qt includes

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QString>
#include <QRect>

namespace Digikam
{

class Q_DECL_HIDDEN BlackFrameToolTip::Private
{
public:

    Private() = default;

public:

    QString          tip;

    QTreeWidget*     view = nullptr;
    QTreeWidgetItem* item = nullptr;
};

BlackFrameToolTip::BlackFrameToolTip(QTreeWidget* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

BlackFrameToolTip::~BlackFrameToolTip()
{
    delete d;
}

void BlackFrameToolTip::setToolTipString(const QString& tip)
{
    d->tip = tip;
}

void BlackFrameToolTip::setItem(QTreeWidgetItem* const item)
{
    d->item = item;

    if (!d->item)
    {
        hide();
    }
    else
    {
        show();
    }
}

void BlackFrameToolTip::show()
{
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        DItemToolTip::show();
    }
}

QRect BlackFrameToolTip::repositionRect()
{
    if (!d->item)
    {
        return QRect();
    }

    QRect rect = d->view->visualItemRect(d->item);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString BlackFrameToolTip::tipContents()
{
    return d->tip;
}

} // namespace Digikam

#include "moc_blackframetooltip.cpp"
