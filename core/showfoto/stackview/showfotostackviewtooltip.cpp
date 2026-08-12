/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Tool tip for Showfoto stack view item.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewtooltip.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfotosettings.h"
#include "showfotostackviewlist.h"
#include "showfototooltipfiller.h"
#include "showfotostackviewitem.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewToolTip::Private
{
public:

    Private() = default;

    ShowfotoStackViewList* view     = nullptr;
    QModelIndex            index;
};

ShowfotoStackViewToolTip::ShowfotoStackViewToolTip(ShowfotoStackViewList* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

ShowfotoStackViewToolTip::~ShowfotoStackViewToolTip()
{
    delete d;
}

void ShowfotoStackViewToolTip::setIndex(const QModelIndex& index)
{
    d->index = index;

    if (!d->index.isValid() || !ShowfotoSettings::instance()->getShowToolTip())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

QRect ShowfotoStackViewToolTip::repositionRect()
{
    if (!d->index.isValid())
    {
        return QRect();
    }

    QRect rect = d->view->visualRect(d->index);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString ShowfotoStackViewToolTip::tipContents()
{
    if (!d->index.isValid())
    {
        return QString();
    }

    ShowfotoItemInfo iteminfo = d->view->infoFromIndex(d->index);

    if (iteminfo.isNull())
    {
        return QString();
    }

    return ShowfotoToolTipFiller::ShowfotoItemInfoTipContents(iteminfo);
}

} // namespace ShowFoto

#include "moc_showfotostackviewtooltip.cpp"
