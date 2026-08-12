/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * SPDX-FileCopyrightText: 2005      by Owen Hirst <n8rider@sbcglobal.net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ratingwidget_p.h"

namespace Digikam
{

RatingBox::RatingBox(QWidget* const parent)
    : DVBox(parent),
      d    (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->ratingWidget = new RatingWidget(this);
    d->ratingWidget->setTracking(false);

    d->shortcut     = new DAdjustableLabel(this);
    QFont fnt       = d->shortcut->font();
    fnt.setItalic(true);
    d->shortcut->setFont(fnt);
    d->shortcut->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->shortcut->setWordWrap(false);

    setContentsMargins(QMargins());
    setSpacing(0);

    // -------------------------------------------------------------

    connect(d->ratingWidget, SIGNAL(signalRatingModified(int)),
            this, SLOT(slotUpdateDescription(int)));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalRatingChanged(int)));
}

RatingBox::~RatingBox()
{
    delete d;
}

void RatingBox::slotUpdateDescription(int rating)
{
    DXmlGuiWindow* const app = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());

    if (app)
    {
        QAction* const ac = app->actionCollection()->action(QString::fromLatin1("rateshortcut-%1").arg(rating));

        if (ac)
        {
            d->shortcut->setAdjustedText(ac->shortcut().toString());
        }
    }
}

} // namespace Digikam
