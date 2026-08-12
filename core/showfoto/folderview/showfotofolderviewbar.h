/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : a tool bar for Showfoto folder view
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QList>
#include <QAction>

// Local includes

#include "dlayoutbox.h"
#include "dpluginaction.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoFolderViewList;
class ShowfotoFolderViewSideBar;

class ShowfotoFolderViewBar : public DVBox
{
    Q_OBJECT

public:

    enum FolderViewTypeMime
    {
        TYPE_MIME_JPEG = 0,
        TYPE_MIME_TIFF,
        TYPE_MIME_PNG,
        TYPE_MIME_PGF,
        TYPE_MIME_HEIF,
        TYPE_MIME_AVIF,
        TYPE_MIME_JXL,
        TYPE_MIME_WEBP,
        TYPE_MIME_DNG,
        TYPE_MIME_RAW,
        TYPE_MIME_NORAW,
        TYPE_MIME_ALL
    };

public:

    explicit ShowfotoFolderViewBar(ShowfotoFolderViewSideBar* const parent);
    ~ShowfotoFolderViewBar()                          override;

    void setIconSize(int size);
    int  iconSize()                             const;

    void setFolderViewMode(int mode);
    int  folderViewMode()                       const;

    void setFolderViewTypeMime(int mime);
    int  folderViewTypeMime()                   const;

    void setBookmarksVisible(bool b);
    bool bookmarksVisible()                     const;

    void setCurrentPath(const QString& path);
    QString currentPath()                       const;

    QAction* toolBarAction(const QString& name) const;
    QList<QAction*> pluginActions()             const;

    void registerPluginActions(const QList<DPluginAction*>& actions);

public Q_SLOTS:

    void slotPreviousEnabled(bool);
    void slotNextEnabled(bool);

Q_SIGNALS:

    void signalTypeMimesChanged(const QString&);
    void signalViewModeChanged(int);
    void signalShowBookmarks(bool);
    void signalSetup();
    void signalIconSizeChanged(int);
    void signalGoHome();
    void signalGoUp();
    void signalGoPrevious();
    void signalGoNext();
    void signalCustomPathChanged(const QString&);
    void signalLoadContents();
    void signalAppendContents();
    void signalPluginActionTriggered(QAction*);

private Q_SLOTS:

    void slotCustomPathChanged();
    void slotIconSizeChanged(int);
    void slotOptionsChanged(QAction*);
    void slotTypeMimesChanged(int index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
