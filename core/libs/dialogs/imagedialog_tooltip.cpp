/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog - List view tooltip.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagedialog_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageDialogToolTip::Private
{
public:

    Private() = default;

public:

    QAbstractItemView*     view    = nullptr;
    QModelIndex            index;
    QUrl                   url;
    QImage                 preview;
    ThumbnailImageCatcher* catcher = nullptr;
    ThumbnailLoadThread*   thread  = nullptr;
};

ImageDialogToolTip::ImageDialogToolTip()
    : DItemToolTip(),
      d           (new Private)
{
    d->thread  = new ThumbnailLoadThread;
    d->thread->setThumbnailSize(64);
    d->thread->setPixmapRequested(false);
    d->catcher = new ThumbnailImageCatcher(d->thread);
}

ImageDialogToolTip::~ImageDialogToolTip()
{
    d->catcher->thread()->stopAllTasks();
    d->catcher->cancel();

    delete d->catcher->thread();
    delete d->catcher;
    delete d;
}

void ImageDialogToolTip::setData(QAbstractItemView* const view,
                                 const QModelIndex& index,
                                 const QUrl& url)
{
    d->view    = view;
    d->index   = index;
    d->url     = url;

    d->catcher->setActive(true);
    d->catcher->thread()->find(ThumbnailIdentifier(d->url.toLocalFile()));
    d->catcher->enqueue();
    QList<QImage> images = d->catcher->waitForThumbnails();

    if (!images.isEmpty())
    {
        d->preview = images.first();
    }
    else
    {
        d->preview = QImage();
    }

    d->catcher->setActive(false);

    if (!d->index.isValid())
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

QRect ImageDialogToolTip::repositionRect()
{
    if (!d->index.isValid())
    {
        return QRect();
    }

    QRect rect = d->view->visualRect(d->index);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString ImageDialogToolTip::tipContents()
{
    if (!d->index.isValid())
    {
        return QString();
    }

    QString identify = ImageDialogPreview::identifyItem(d->url, d->preview);

    return identify;
}

} // namespace Digikam
