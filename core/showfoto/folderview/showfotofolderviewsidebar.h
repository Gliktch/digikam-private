/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Side Bar Widget for the Showfoto folder view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QListView>
#include <QPixmap>
#include <QWidget>
#include <QEvent>
#include <QAction>
#include <QStringList>
#include <QModelIndex>

// Local includes

#include "statesavingobject.h"
#include "showfotoitemsortsettings.h"
#include "dpluginaction.h"

using namespace Digikam;

namespace ShowFoto
{

class Showfoto;

class ShowfotoFolderViewSideBar : public QWidget,
                                  public StateSavingObject
{
    Q_OBJECT

public:

    explicit ShowfotoFolderViewSideBar(Showfoto* const parent);
    ~ShowfotoFolderViewSideBar()              override;

    void          doLoadState()               override;
    void          doSaveState()               override;
    const QIcon   getIcon();
    const QString getCaption();

    QString currentFolder()             const;
    QString currentPath()               const;

    void setCurrentPath(const QString& newPathNative);
    void setCurrentPathWithoutUndo(const QString& newPath);

    void setSortOrder(int order);
    void setSortRole(int role);

    void loadContents(const QModelIndex& index, bool append = false);

    void registerPluginActions(const QList<DPluginAction*>& actions);
    QList<QAction*> pluginActions()     const;

public Q_SLOTS:

    void slotUndo();
    void slotRedo();
    void slotGoHome();
    void slotGoUp();
    void slotLoadContents();
    void slotAppendContents();

private Q_SLOTS:

    void slotCustomPathChanged(const QString&);
    void slotViewModeChanged(int);
    void slotShowBookmarks(bool);
    void slotPluginActionTriggered(QAction*);
    void slotTypeMimesChanged(const QString& patterns);

Q_SIGNALS:

    void signalAddBookmark();
    void signalSetup();
    void signalLoadContentsFromPath(const QString& path);
    void signalLoadContentsFromFiles(const QStringList& files, const QString& current);
    void signalAppendContentsFromFiles(const QStringList& files, const QString& current);

private:

    ShowfotoFolderViewSideBar(QWidget*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
