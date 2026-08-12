/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the gps search.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpssearchsidebarwidget.h"

// Qt includes

#include <QScrollArea>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "albumpointer.h"
#include "applicationsettings.h"
#include "editablesearchtreeview.h"
#include "searchfolderview.h"
#include "searchtabheader.h"
#include "searchtextbar.h"
#include "searchtreeview.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSSearchSideBarWidget::Private
{
public:

    Private() = default;

    GPSSearchView* gpsSearchView = nullptr;
};

GPSSearchSideBarWidget::GPSSearchSideBarWidget(QWidget* const parent,
                                               SearchModel* const searchModel,
                                               SearchModificationHelper* const searchModificationHelper,
                                               ItemFilterModel* const imageFilterModel,
                                               QItemSelectionModel* const itemSelectionModel)
    : SidebarWidget(parent),
      d            (new Private)
{
    setObjectName(QLatin1String("GPS Search Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F8));

    d->gpsSearchView = new GPSSearchView(this, searchModel, searchModificationHelper, imageFilterModel, itemSelectionModel);
    d->gpsSearchView->setConfigGroup(getConfigGroup());

    const int spacing             = layoutSpacing();

    QScrollArea* const scrollArea = new QScrollArea(this);
    QVBoxLayout* const layout     = new QVBoxLayout(this);

    layout->addWidget(scrollArea);
    layout->setContentsMargins(0, 0, spacing, 0);
    scrollArea->setWidget(d->gpsSearchView);
    scrollArea->setWidgetResizable(true);

    connect(d->gpsSearchView, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)),
            this, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)));
}

GPSSearchSideBarWidget::~GPSSearchSideBarWidget()
{
    delete d;
}

void GPSSearchSideBarWidget::setActive(bool active)
{
    d->gpsSearchView->setActive(active);
}

void GPSSearchSideBarWidget::doLoadState()
{
    d->gpsSearchView->loadState();
}

void GPSSearchSideBarWidget::doSaveState()
{
    d->gpsSearchView->saveState();
}

void GPSSearchSideBarWidget::applySettings()
{
}

void GPSSearchSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    d->gpsSearchView->changeAlbumFromHistory(dynamic_cast<SAlbum*>(album.first()));
}

const QIcon GPSSearchSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("globe"));
}

const QString GPSSearchSideBarWidget::getCaption()
{
    return i18nc("Search images on a map", "Map");
}

} // namespace Digikam

#include "moc_gpssearchsidebarwidget.cpp"
