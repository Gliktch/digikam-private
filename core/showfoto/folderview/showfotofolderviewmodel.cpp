/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view model.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewmodel.h"

// Qt includes

#include <QPainter>
#include <QIODevice>
#include <QModelIndex>
#include <QMimeDatabase>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "drawdecoder.h"
#include "thumbnailloadthread.h"
#include "showfotofolderviewlist.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewModel::Private
{

public:

    Private() = default;

    ThumbnailLoadThread*    thumbnailThread = nullptr;
    ShowfotoFolderViewList* view            = nullptr;

    static const int        s_maxSize;      ///< Max icon size.
};

const int ShowfotoFolderViewModel::Private::s_maxSize = 256;

ShowfotoFolderViewModel::ShowfotoFolderViewModel(ShowfotoFolderViewList* const view)
    : QFileSystemModel(view),
      d               (new Private)
{
    d->view               = view;

    setObjectName(QLatin1String("ShowfotoFolderViewModel"));

    // --- Populate the model

    setRootPath(QDir::rootPath());

    // If an item fails the filter, hide it

    setNameFilterDisables(false);

    d->thumbnailThread = new ThumbnailLoadThread;
    d->thumbnailThread->setSendSurrogatePixmap(false);
    d->thumbnailThread->setThumbnailSize(Private::s_maxSize);

    connect(d->thumbnailThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(refreshThumbnails(LoadingDescription,QPixmap)));
}

ShowfotoFolderViewModel::~ShowfotoFolderViewModel()
{
    d->thumbnailThread->stopAllTasks();

    delete d->thumbnailThread;
    delete d;
}

int ShowfotoFolderViewModel::maxIconSize()
{
    return (Private::s_maxSize);
}

QVariant ShowfotoFolderViewModel::data(const QModelIndex& index, int role) const
{
    if ((role == Qt::DecorationRole) && (index.column() == 0))
    {
        QString path = fileInfo(index).absoluteFilePath();

        if (isReadableImageFile(path))
        {
            QPixmap pix;

            if (d->thumbnailThread->find(ThumbnailIdentifier(path), pix))
            {
                pix = pix.scaled(d->view->iconSize(), Qt::KeepAspectRatio,
                                                      Qt::FastTransformation);

                QPixmap icon(d->view->iconSize());
                icon.fill(Qt::transparent);
                QPainter p(&icon);
                p.drawPixmap((icon.width()  - pix.width() ) / 2,
                             (icon.height() - pix.height()) / 2,
                             pix);

                return icon;
            }
        }
    }

    return QFileSystemModel::data(index, role);
}

void ShowfotoFolderViewModel::refreshThumbnails(const LoadingDescription& desc, const QPixmap& pix)
{
    QModelIndex current = index(desc.filePath);

    if (current.isValid() && !pix.isNull())
    {
        Q_EMIT dataChanged(current, current);
    }
}

QStringList ShowfotoFolderViewModel::currentFilesPath() const
{
    QStringList paths;
    QModelIndex idx = index(rootPath());
    QModelIndex child;
    QString file;

    for (int i = 0 ; i < rowCount(idx) ; ++i)
    {
        child = index(i, idx.column(), idx);
        file  = filePath(child);

        if (QFileInfo(file).isFile())
        {
            paths << file;
        }
    }

    return paths;
}

} // namespace ShowFoto

#include "moc_showfotofolderviewmodel.cpp"
