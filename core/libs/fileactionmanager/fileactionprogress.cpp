/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-23
 * Description : file action progress indicator
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fileactionprogress.h"

// Qt includes

#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

FileActionProgress::FileActionProgress(const QString& name)
    : ProgressItem(nullptr, name, QString(), QString(), true, true)
{
    ProgressManager::addProgressItem(this);
    setLabel(i18n("Process Items"));
    setThumbnail(QIcon::fromTheme(QLatin1String("digikam")));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));
}

void FileActionProgress::slotProgressValue(float v)
{
    setProgress((int)(v*100.0));
}

void FileActionProgress::slotProgressStatus(const QString& st)
{
    setStatus(st);
}

void FileActionProgress::slotCompleted()
{
    Q_EMIT signalComplete();

    setComplete();
}

void FileActionProgress::slotCancel()
{
    setComplete();
}

} // namespace Digikam

#include "moc_fileactionprogress.cpp"
