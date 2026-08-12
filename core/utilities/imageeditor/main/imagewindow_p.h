/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "imagewindow.h"

// C++ includes

#include <cstdio>
#include <vector>
#include <algorithm>

// Qt includes

#include <QKeySequence>
#include <QCloseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPixmap>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>
#include <QScopedPointer>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kxmlgui_version.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "versionmanager.h"
#include "dlayoutbox.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "applicationsettings.h"
#include "canvas.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "componentsinfodlg.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "coredbchangesets.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "dimg.h"
#include "editorcore.h"
#include "dimagehistory.h"
#include "digikamapp.h"
#include "dio.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "iccsettingscontainer.h"
#include "itemattributeswatch.h"
#include "itemfiltermodel.h"
#include "itemdragdrop.h"
#include "itemdescedittab.h"
#include "iteminfo.h"
#include "itemlistmodel.h"
#include "itempropertiessidebardb.h"
#include "itempropertiesversionstab.h"
#include "itemscanner.h"
#include "itemthumbnailbar.h"
#include "iofilesettings.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metaenginesettings.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "ratingwidget.h"
#include "savingcontext.h"
#include "scancontroller.h"
#include "setup.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tagsactionmngr.h"
#include "tagscache.h"
#include "tagspopupmenu.h"
#include "tagregion.h"
#include "thememanager.h"
#include "thumbbardock.h"
#include "thumbnailloadthread.h"
#include "undostate.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"
#include "facetagseditor.h"

namespace Digikam
{

class Q_DECL_HIDDEN DatabaseVersionManager : public VersionManager
{
public:

    QString toplevelDirectory(const QString& path) override
    {
        CollectionLocation loc = CollectionManager::instance()->locationForPath(path);

        if (!loc.isNull())
        {
            return loc.albumRootPath();
        }

        return QLatin1String("/");
    }
};

// -----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ImageWindow::Private
{

public:

    Private() = default;

    QModelIndex currentIndex() const
    {
        return imageFilterModel->indexForItemInfo(currentItemInfo);
    }

    QModelIndex currentSourceIndex() const
    {
        return imageInfoModel->indexForItemInfo(currentItemInfo);
    }

    bool currentIsValid() const
    {
        return !currentItemInfo.isNull();
    }

    QUrl currentUrl() const
    {
        return currentItemInfo.fileUrl();
    }

    QModelIndex nextIndex() const
    {
        return imageFilterModel->index(currentIndex().row() + 1, 0);
    }

    QModelIndex previousIndex() const
    {
        return imageFilterModel->index(currentIndex().row() - 1, 0);
    }

    QModelIndex firstIndex() const
    {
        return imageFilterModel->index(0, 0);
    }

    QModelIndex lastIndex() const
    {
        return imageFilterModel->index(imageFilterModel->rowCount() - 1, 0);
    }

    ItemInfo imageInfo(const QModelIndex& index) const
    {
        return imageFilterModel->imageInfo(index);
    }

    void setThumbBarToCurrent()
    {
        QModelIndex index = imageFilterModel->indexForItemInfo(currentItemInfo);

        if (index.isValid())
        {
            thumbBar->setCurrentIndex(index);
        }
        else
        {
            thumbBar->setCurrentWhenAvailable(currentItemInfo.id());
        }
    }

    void ensureModelContains(const ItemInfo& info)
    {
        if (!imageInfoModel->hasImage(info))
        {
            imageInfoModel->addItemInfoSynchronously(info);
            imageFilterModel->sort(imageFilterModel->sortColumn());
        }
    }

public:

    const QString                configShowThumbbarEntry                = QLatin1String("Show Thumbbar");

    QMainWindow*                 viewContainer                          = nullptr;

    QAction*                     toMainWindowAction                     = nullptr;

    /// Delete actions
    QAction*                     fileDeletePermanentlyAction            = nullptr;
    QAction*                     fileDeletePermanentlyDirectlyAction    = nullptr;
    QAction*                     fileTrashDirectlyAction                = nullptr;

    ItemInfo                     currentItemInfo;
    ItemListModel*               imageInfoModel                         = nullptr;
    ItemFilterModel*             imageFilterModel                       = nullptr;
    ItemDragDropHandler*         dragDropHandler                        = nullptr;

    ItemThumbnailBar*            thumbBar                               = nullptr;
    ThumbBarDock*                thumbBarDock                           = nullptr;

    ItemPropertiesSideBarDB*     rightSideBar                           = nullptr;

    DatabaseVersionManager       versionManager;

    QList<FaceTagsIface>         facesList;
};

} // namespace Digikam
