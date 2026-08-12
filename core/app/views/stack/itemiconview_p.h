/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of item icon view interface.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemiconview.h"

// Qt includes

#include <QTimer>
#include <QShortcut>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "albumhistory.h"
#include "labelstreeview.h"
#include "albumpointer.h"
#include "coredbsearchxml.h"
#include "dbinfoiface.h"
#include "digikamapp.h"
#include "digikamitemview.h"
#include "dfileoperations.h"
#include "dmessagebox.h"
#include "dzoombar.h"
#include "dtrashitemmodel.h"
#include "facescansettings.h"
#include "fileactionmngr.h"
#include "fileactionprogress.h"
#include "filtersidebarwidget.h"
#include "filterstatusbar.h"
#include "itemalbummodel.h"
#include "itemdescedittab.h"
#include "itempreviewview.h"
#include "itempropertiessidebardb.h"
#include "itempropertiesversionstab.h"
#include "itemthumbnailbar.h"
#include "itemviewutilities.h"
#include "albumfolderviewsidebarwidget.h"
#include "datefolderviewsidebarwidget.h"
#include "timelinesidebarwidget.h"
#include "searchsidebarwidget.h"
#include "fuzzysearchsidebarwidget.h"
#include "labelssidebarwidget.h"
#include "peoplesidebarwidget.h"
#include "tagviewsidebarwidget.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metaenginesettings.h"
#include "metadatasynchronizer.h"
#include "metadataremover.h"
#include "metadatahubmngr.h"
#include "newitemsfinder.h"
#include "queuemgrwindow.h"
#include "scancontroller.h"
#include "surveywindow.h"
#include "setup.h"
#include "sidebar.h"
#include "statusprogressbar.h"
#include "tableview.h"
#include "tagmodificationhelper.h"
#include "tagsactionmngr.h"
#include "tagscache.h"
#include "tagsmanager.h"
#include "thumbsgenerator.h"
#include "trashview.h"
#include "versionmanagersettings.h"
#include "contextmenuhelper.h"
#include "albumlabelssearchhandler.h"
#include "dnotificationwidget.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "addtagslineedit.h"
#include "tagfolderview.h"

#ifdef HAVE_MEDIAPLAYER
#   include "itempreviewvideo.h"
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_GEOLOCATION
#   include "gpssearchsidebarwidget.h"
#   include "mapwidgetview.h"
#endif // HAVE_GEOLOCATION

namespace Digikam
{

class Q_DECL_HIDDEN ItemIconView::Private
{
public:

    Private() = default;

public:

    QString userPresentableAlbumTitle(const QString& title) const
    {
        if      (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch))
        {
            return i18n("Fuzzy Sketch Search");
        }
        else if (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))
        {
            return i18n("Fuzzy Image Search");
        }
        else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch))
        {
            return i18n("Map Search");
        }
        else if (
                 (title == SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch)) ||
                 (title == SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch))
                )
        {
            return i18n("Last Search");
        }
        else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch))
        {
            return i18n("Timeline");
        }

        return title;
    }

    void addPageUpDownActions(const ItemIconView* const q, QWidget* const w)
    {
        defineShortcut(w, Qt::Key_PageDown, q, SLOT(slotNextItem()));
        defineShortcut(w, Qt::Key_Down,     q, SLOT(slotNextItem()));
        defineShortcut(w, Qt::Key_Right,    q, SLOT(slotNextItem()));

        defineShortcut(w, Qt::Key_PageUp,   q, SLOT(slotPrevItem()));
        defineShortcut(w, Qt::Key_Up,       q, SLOT(slotPrevItem()));
        defineShortcut(w, Qt::Key_Left,     q, SLOT(slotPrevItem()));
    }

public:

    bool                          useAlbumHistory           = false;

    int                           initialAlbumID            = 0;
    int                           thumbSize                 = ThumbnailSize::Medium;

    QMainWindow*                  dockArea                  = nullptr;

    SidebarSplitter*              splitter                  = nullptr;

    QTimer*                       selectionTimer            = nullptr;
    QTimer*                       thumbSizeTimer            = nullptr;
    QTimer*                       msgNotifyTimer            = nullptr;

    // left side bar

    AlbumFolderViewSideBarWidget* albumFolderSideBar        = nullptr;
    TagViewSideBarWidget*         tagViewSideBar            = nullptr;
    LabelsSideBarWidget*          labelsSideBar             = nullptr;
    DateFolderViewSideBarWidget*  dateViewSideBar           = nullptr;
    TimelineSideBarWidget*        timelineSideBar           = nullptr;
    SearchSideBarWidget*          searchSideBar             = nullptr;
    FuzzySearchSideBarWidget*     fuzzySearchSideBar        = nullptr;

#ifdef HAVE_GEOLOCATION

    GPSSearchSideBarWidget*       gpsSearchSideBar          = nullptr;
    MapWidgetView*                mapView                   = nullptr;

#endif // HAVE_GEOLOCATION

    PeopleSideBarWidget*          peopleSideBar             = nullptr;
    DigikamApp*                   parent                    = nullptr;
    DigikamItemView*              iconView                  = nullptr;
    TableView*                    tableView                 = nullptr;
    TrashView*                    trashView                 = nullptr;
    ItemViewUtilities*            utilities                 = nullptr;
    AlbumManager*                 albumManager              = nullptr;
    AlbumHistory*                 albumHistory              = nullptr;
    StackedView*                  stackedView               = nullptr;
    StackedView::StackedViewMode  lastViewMode              = StackedView::IconViewMode;

    AlbumModificationHelper*      albumModificationHelper   = nullptr;
    TagModificationHelper*        tagModificationHelper     = nullptr;
    SearchModificationHelper*     searchModificationHelper  = nullptr;

    Sidebar*                      leftSideBar               = nullptr;
    ItemPropertiesSideBarDB*      rightSideBar              = nullptr;

    FilterSideBarWidget*          filterWidget              = nullptr;

    QList<SidebarWidget*>         leftSideBarWidgets;

    DModelFactory*                modelCollection           = nullptr;
    AlbumLabelsSearchHandler*     labelsSearchHandler       = nullptr;
    DNotificationWidget*          errorWidget               = nullptr;
};

} // namespace Digikam
