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

#pragma once

// Qt includes

#include <QString>
#include <QPixmap>
#include <QTreeWidget>
#include <QIcon>

namespace Digikam
{

class CameraFolderItem;

class CameraFolderView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit CameraFolderView(QWidget* const parent);
    ~CameraFolderView() override;

    void addVirtualFolder(const QString& name, const QIcon &icon = QIcon::fromTheme(QLatin1String("camera-photo")));
    void addRootFolder(const QString& folder, int nbItems = -1, const QIcon& icon = QIcon::fromTheme(QLatin1String("folder")));

    CameraFolderItem* addFolder(const QString& folder, const QString& subFolder, int nbItems,
                                const QIcon& icon = QIcon::fromTheme(QLatin1String("folder")));

    CameraFolderItem* findFolder(const QString& folderPath);

    CameraFolderItem* virtualFolder() const;
    CameraFolderItem* rootFolder()    const;

    virtual void clear();

Q_SIGNALS:

    void signalFolderChanged(CameraFolderItem*);
    void signalCleared();

private Q_SLOTS:

    void slotCurrentChanged(QTreeWidgetItem*, int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
