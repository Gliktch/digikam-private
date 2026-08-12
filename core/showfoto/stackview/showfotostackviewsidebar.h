/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Side Bar Widget for the Showfoto stack view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QList>
#include <QUrl>
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

class ShowfotoThumbnailBar;
class Showfoto;

class ShowfotoStackViewSideBar : public QWidget,
                                 public StateSavingObject
{
    Q_OBJECT

public:

    explicit ShowfotoStackViewSideBar(Showfoto* const parent);
    ~ShowfotoStackViewSideBar()               override;

    void setThumbbar(ShowfotoThumbnailBar* const thumbbar);

    void          doLoadState()               override;
    void          doSaveState()               override;
    const QIcon   getIcon();
    const QString getCaption();

    void setSortOrder(int order);
    int sortOrder()                     const;

    void setSortRole(int role);
    int sortRole()                      const;

    void registerPluginActions(const QList<DPluginAction*>& actions);
    QList<QAction*> pluginActions()     const;

    QList<QUrl> urls()                  const;
    QUrl currentUrl()                   const;

    int iconSize()                      const;

private Q_SLOTS:

    void slotPluginActionTriggered();

Q_SIGNALS:

    void signalShowfotoItemInfoActivated(const ShowfotoItemInfo& info);
    void signalLoadContentsFromFiles(const QStringList& files, const QString& current);
    void signalAddFavorite();
    void signalClearItemsList();
    void signalRemoveItemInfos(const QList<ShowfotoItemInfo>& infos);

private:

    ShowfotoStackViewSideBar(QWidget*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
