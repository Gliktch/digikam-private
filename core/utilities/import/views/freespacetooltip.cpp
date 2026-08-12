/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-01-19
 * Description : free space widget tool tip
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "freespacetooltip.h"

// Qt includes

#include <QWidget>
#include <QString>
#include <QRect>

namespace Digikam
{

class Q_DECL_HIDDEN FreeSpaceToolTip::Private
{
public:

    Private() = default;

    QString  tip;
    QWidget* parent = nullptr;
};

FreeSpaceToolTip::FreeSpaceToolTip(QWidget* const parent)
    : DItemToolTip(),
      d           (new Private)
{
    d->parent = parent;
}

FreeSpaceToolTip::~FreeSpaceToolTip()
{
    delete d;
}

void FreeSpaceToolTip::setToolTip(const QString& tip)
{
    d->tip = tip;
}

void FreeSpaceToolTip::show()
{
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        DItemToolTip::show();
    }
}

QRect FreeSpaceToolTip::repositionRect()
{
    if (!d->parent)
    {
        return QRect();
    }

    QRect rect = d->parent->rect();
    rect.moveTopLeft(d->parent->mapToGlobal(rect.topLeft()));

    return rect;
}

QString FreeSpaceToolTip::tipContents()
{
    return d->tip;
}

} // namespace Digikam

#include "moc_freespacetooltip.cpp"
