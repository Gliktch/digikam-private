/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mapwidget_p.h"

namespace Digikam
{

MapWidget::MapWidget(QWidget* const parent)
    : QWidget(parent),
      s      (new GeoIfaceSharedData),
      d      (new Private)
{
    createActions();

    s->worldMapWidget    = this;
    s->tileGrouper       = new TileGrouper(s, this);

    d->stackedLayout     = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->placeholderWidget = new PlaceholderWidget();
    d->stackedLayout->addWidget(d->placeholderWidget);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));

#ifdef HAVE_GEOLOCATION

    BackendMarble* const bm = new BackendMarble(s, this);
    d->loadedBackends.append(bm);
    bm->mapWidget();        // preload plugins.

#endif

/*
    d->loadedBackends.append(new BackendOSM(s, this));
*/
    createActionsForBackendSelection();

    setAcceptDrops(true);
}

MapWidget::~MapWidget()
{
    // release all widgets:

    for (int i = 0 ; i < d->stackedLayout->count() ; ++i)
    {
        d->stackedLayout->removeWidget(d->stackedLayout->widget(i));
    }

    GeolocationSettings::instance()->unregisterWidget(this);

    qDeleteAll(d->loadedBackends);
    d->currentBackend = nullptr;
    d->loadedBackends.clear();

    delete d;

    /// @todo delete s, but make sure it is not accessed by any other objects any more!
}

} // namespace Digikam

#include "moc_mapwidget.cpp"
