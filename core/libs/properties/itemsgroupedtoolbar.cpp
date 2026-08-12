/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A list view to show grouped items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemsgroupedtoolbar.h"

// Qt includes

#include <QIcon>
#include <QToolButton>
#include <QHBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemsgroupedview.h"

namespace Digikam
{

ItemsGroupedToolBar::ItemsGroupedToolBar(ItemsGroupedView* const view)
    : QWidget(view)
{
    QHBoxLayout* const hlay         = new QHBoxLayout(this);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(2);
    hlay->setSizeConstraint(QLayout::SetMinimumSize);

    QToolButton* const deleteButton = new QToolButton(this);
    deleteButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    deleteButton->setToolTip(i18n("Remove from Group"));

    connect(deleteButton, &QToolButton::clicked,
            this, &ItemsGroupedToolBar::signalDeleteRequested);

    hlay->addWidget(deleteButton);
    adjustSize();
    setMinimumSize(minimumSizeHint());

    hide();
}

} // namespace Digikam

#include "moc_itemsgroupedtoolbar.cpp"
