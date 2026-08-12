/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a list of camera folders.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "camerafolderview.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "camerafolderitem.h"

namespace Digikam
{

class Q_DECL_HIDDEN CameraFolderView::Private
{
public:

    Private() = default;

    QString           cameraName    = QLatin1String("Camera");

    CameraFolderItem* virtualFolder = nullptr;
    CameraFolderItem* rootFolder    = nullptr;
};

CameraFolderView::CameraFolderView(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setColumnCount(1);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setDragEnabled(false);
    setAcceptDrops(false);
    setDropIndicatorShown(false);
    setHeaderLabels(QStringList() << i18n("Camera Folders"));

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotCurrentChanged(QTreeWidgetItem*,int)));

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotCurrentChanged(QTreeWidgetItem*,int)));
}

CameraFolderView::~CameraFolderView()
{
    delete d;
}

void CameraFolderView::addVirtualFolder(const QString& name, const QIcon& icon)
{
    d->cameraName    = name;
    d->virtualFolder = new CameraFolderItem(this, d->cameraName, icon);
    d->virtualFolder->setExpanded(true);
    d->virtualFolder->setSelected(false);

    // item is not selectable.

    d->virtualFolder->setFlags(d->virtualFolder->flags() & (int)!Qt::ItemIsSelectable);
    d->virtualFolder->setDisabled(false);
}

void CameraFolderView::addRootFolder(const QString& folder, int nbItems, const QIcon &icon)
{
    d->rootFolder = new CameraFolderItem(d->virtualFolder, folder, folder, icon);
    d->rootFolder->setExpanded(true);

    if (nbItems != -1)
    {
        d->rootFolder->setCount(nbItems);
    }
}

CameraFolderItem* CameraFolderView::addFolder(const QString& folder, const QString& subFolder,
                                              int nbItems, const QIcon &icon)
{
    CameraFolderItem* const parentItem = findFolder(folder);

    if (parentItem)
    {
        QString path(folder);

        if (!folder.endsWith(QLatin1Char('/')))
        {
            path += QLatin1Char('/');
        }

        path += subFolder + QLatin1Char('/');

        if (!findFolder(path))
        {
            CameraFolderItem* item = new CameraFolderItem(parentItem, subFolder, path, icon);

            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Adding subfolder:" << subFolder
                                          << "Folder path:" << item->folderPath();

            item->setCount(nbItems);
            item->setExpanded(true);

            return item;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Could not find parent for subfolder"
                                        << subFolder << "of folder" << folder;
        return nullptr;
    }
}

CameraFolderItem* CameraFolderView::findFolder(const QString& folderPath)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        CameraFolderItem* const lvItem = dynamic_cast<CameraFolderItem*>(*it);

        if (lvItem && (lvItem->folderPath() == folderPath))
        {
            return lvItem;
        }

        ++it;
    }

    return nullptr;
}

void CameraFolderView::slotCurrentChanged(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        Q_EMIT signalFolderChanged(nullptr);
    }
    else
    {
        Q_EMIT signalFolderChanged(dynamic_cast<CameraFolderItem*>(item));
    }
}

CameraFolderItem* CameraFolderView::virtualFolder() const
{
    return d->virtualFolder;
}

CameraFolderItem* CameraFolderView::rootFolder() const
{
    return d->rootFolder;
}

void CameraFolderView::clear()
{
    QTreeWidget::clear();
    d->virtualFolder = nullptr;
    d->rootFolder    = nullptr;

    Q_EMIT signalCleared();
}

} // namespace Digikam

#include "moc_camerafolderview.cpp"
