/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorlabelwidget_p.h"

namespace Digikam
{

ColorLabelSelector::ColorLabelSelector(QWidget* parent)
    : QPushButton(parent),
      d          (new Private)
{
    QMenu* const popup          = new QMenu(this);
    setMenu(popup);

    QWidgetAction* const action = new QWidgetAction(this);
    d->clw                      = new ColorLabelWidget(this);
    action->setDefaultWidget(d->clw);
    popup->addAction(action);
    slotColorLabelChanged(NoColorLabel);

    connect(d->clw, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotColorLabelChanged(int)));
}

ColorLabelSelector::~ColorLabelSelector()
{
    delete d;
}

ColorLabelWidget* ColorLabelSelector::colorLabelWidget() const
{
    return d->clw;
}

void ColorLabelSelector::setColorLabel(ColorLabel label)
{
    blockSignals(true);

    d->clw->setColorLabels(QList<ColorLabel>() << label);
    slotColorLabelChanged(label);

    blockSignals(false);
}

ColorLabel ColorLabelSelector::colorLabel()
{
    QList<ColorLabel> list = d->clw->colorLabels();

    if (!list.isEmpty())
    {
        return list.first();
    }

    return NoColorLabel;
}

void ColorLabelSelector::slotColorLabelChanged(int id)
{
    setText(QString());
    setIcon(d->clw->buildIcon((ColorLabel)id));

    DXmlGuiWindow* app = nullptr;
    const auto wdgs    = qApp->topLevelWidgets();

    for (QWidget* const widget : wdgs)
    {
        app = dynamic_cast<DXmlGuiWindow*>(widget);

        if (app)
        {
            break;
        }
    }

    if (app)
    {
        QAction* const ac = app->actionCollection()->action(QString::fromLatin1("colorshortcut-%1").arg(id));

        if (ac)
        {
            setToolTip(ac->toolTip());
        }
    }

    menu()->close();

    Q_EMIT signalColorLabelChanged(id);
}

} // namespace Digikam
