/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-26
 * Description : a progress bar with information dispatched to progress manager
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dprogresswdg.h"

// Qt includes

#include <QString>
#include <QIcon>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DProgressWdg::Private
{
public:

    Private() = default;

    ProgressItem* findProgressItem() const
    {
        return ProgressManager::instance()->findItembyId(progressId);
    }

public:

    QString progressId;
};

DProgressWdg::DProgressWdg(QWidget* const parent)
    : QProgressBar(parent),
      d           (new Private)
{
    connect(this, &DProgressWdg::valueChanged,
            this, &DProgressWdg::slotValueChanged);
}

DProgressWdg::~DProgressWdg()
{
    delete d;
}

void DProgressWdg::slotValueChanged(int)
{
    float percents           = ((float)value() / (float)maximum()) * 100.0;
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setProgress(percents);
    }
}

void DProgressWdg::progressCompleted()
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setComplete();
    }
}

void DProgressWdg::progressThumbnailChanged(const QPixmap& thumb)
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setThumbnail(thumb);
    }
}

void DProgressWdg::progressStatusChanged(const QString& status)
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setStatus(status);
    }
}

void DProgressWdg::progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb)
{
    ProgressItem* const item = ProgressManager::createProgressItem(title,
                                                                   QString(),
                                                                   canBeCanceled,
                                                                   hasThumb);

    if (canBeCanceled)
    {
        connect(item, SIGNAL(progressItemCanceledById(QString)),
                this, SLOT(slotProgressCanceled(QString)));
    }

    d->progressId = item->id();
}

void DProgressWdg::slotProgressCanceled(const QString& id)
{
    if (d->progressId == id)
    {
        Q_EMIT signalProgressCanceled();
    }
}

} // namespace Digikam

#include "moc_dprogresswdg.cpp"
