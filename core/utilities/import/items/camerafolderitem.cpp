/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a camera folder.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "camerafolderitem.h"

namespace Digikam
{

class Q_DECL_HIDDEN CameraFolderItem::Private
{
public:

    Private() = default;

    bool    virtualFolder   = true;
    int     count           = 0;

    QString folderName;
    QString folderPath;
    QString name;
};

CameraFolderItem::CameraFolderItem(QTreeWidget* const parent, const QString& name, const QIcon& icon)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    d->name = name;
    setIcon(0, icon);
    setText(0, d->name);
}

CameraFolderItem::CameraFolderItem(QTreeWidgetItem* const parent, const QString& folderName,
                                   const QString& folderPath, const QIcon &icon)
    : QTreeWidgetItem(parent),
      d              (new Private)
{
    d->folderName    = folderName;
    d->folderPath    = folderPath;
    d->virtualFolder = false;
    d->name          = folderName;
    setIcon(0, icon);
    setText(0, d->name);
}

CameraFolderItem::~CameraFolderItem()
{
    delete d;
}

bool CameraFolderItem::isVirtualFolder() const
{
    return d->virtualFolder;
}

QString CameraFolderItem::folderName() const
{
    return d->folderName;
}

QString CameraFolderItem::folderPath() const
{
    return d->folderPath;
}

void CameraFolderItem::changeCount(int val)
{
    d->count += val;
    setText(0, QString::fromUtf8("%1 (%2)").arg(d->name).arg(d->count));
}

void CameraFolderItem::setCount(int val)
{
    d->count = val;
    setText(0, QString::fromUtf8("%1 (%2)").arg(d->name).arg(d->count));
}

int CameraFolderItem::count() const
{
    return d->count;
}

} // namespace Digikam
