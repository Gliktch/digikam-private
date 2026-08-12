/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-27
 * Description : Widget showing a throbber ("working" animation)
 *
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "workingwidget.h"

// Qt includes

#include <QImage>
#include <QTimer>
#include <QList>
#include <QPixmap>
#include <QIcon>

// Local includes

#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class Q_DECL_HIDDEN WorkingWidget::Private
{

public:

    Private() = default;

    DWorkingPixmap* pixmaps         = nullptr;
    int             currentPixmap   = 0;
    QTimer          timer;
};

WorkingWidget::WorkingWidget(QWidget* const parent)
    : QLabel(parent),
      d     (new Private)
{
    d->pixmaps = new DWorkingPixmap(this);

    connect(&d->timer, SIGNAL(timeout()),
            this, SLOT(slotChangeImage()));

    d->timer.start(100);
    slotChangeImage();
}

WorkingWidget::~WorkingWidget()
{
    delete d;
}

void WorkingWidget::slotChangeImage()
{
    if (d->currentPixmap >= d->pixmaps->frameCount())
    {
        d->currentPixmap = 0;
    }

    setPixmap(d->pixmaps->frameAt(d->currentPixmap));

    d->currentPixmap++;

    Q_EMIT animationStep();
}

void WorkingWidget::toggleTimer(bool turnOn)
{
    if      (turnOn && !d->timer.isActive())
    {
        d->timer.start();
    }
    else if (!turnOn && d->timer.isActive())
    {
        d->timer.stop();
    }
}

} // namespace Digikam

#include "moc_workingwidget.cpp"
