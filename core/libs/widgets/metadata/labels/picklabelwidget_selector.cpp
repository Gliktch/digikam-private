/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "picklabelwidget_p.h"

namespace Digikam
{

PickLabelSelector::PickLabelSelector(QWidget* const parent)
    : QPushButton(parent),
      d          (new Private)
{
    QMenu* const popup          = new QMenu(this);
    setMenu(popup);

    QWidgetAction* const action = new QWidgetAction(this);
    d->plw                      = new PickLabelWidget(this);
    action->setDefaultWidget(d->plw);
    popup->addAction(action);
    slotPickLabelChanged(NoPickLabel);

    connect(d->plw, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotPickLabelChanged(int)));
}

PickLabelSelector::~PickLabelSelector()
{
    delete d;
}

PickLabelWidget* PickLabelSelector::pickLabelWidget() const
{
    return d->plw;
}

void PickLabelSelector::setPickLabel(PickLabel label)
{
    blockSignals(true);

    d->plw->setPickLabels(QList<PickLabel>() << label);
    slotPickLabelChanged(label);

    blockSignals(false);
}

PickLabel PickLabelSelector::pickLabel()
{
    QList<PickLabel> list = d->plw->pickLabels();

    if (!list.isEmpty())
    {
        return list.first();
    }

    return NoPickLabel;
}

void PickLabelSelector::slotPickLabelChanged(int id)
{
    setText(QString());
    setIcon(d->plw->buildIcon((PickLabel)id));
    setToolTip(i18nc("@info: pick label selector", "Pick Label: %1", d->plw->labelPickName((PickLabel)id)));
    menu()->close();

    Q_EMIT signalPickLabelChanged(id);
}

} // namespace Digikam
