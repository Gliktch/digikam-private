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

RatingMenuAction::RatingMenuAction(QMenu* const parent)
    : QMenu(parent)
{
    setTitle(i18n("Rating"));
    QWidgetAction* const wa = new QWidgetAction(this);
    RatingBox* const rb     = new RatingBox(parent);
    wa->setDefaultWidget(rb);
    addAction(wa);

    connect(rb, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalRatingChanged(int)));

    connect(rb, SIGNAL(signalRatingChanged(int)),
            parent, SLOT(close()));
}

} // namespace Digikam
