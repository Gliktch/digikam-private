/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "geomodelhelper.h"
#include "mapwidget.h"
#include "statesavingobject.h"
#include "itemfiltermodel.h"

namespace Digikam
{

class Album;
class SAlbum;
class SearchModel;
class SearchModificationHelper;

class GPSSearchView : public QWidget,
                      public StateSavingObject
{
    Q_OBJECT

public:

    explicit GPSSearchView(QWidget* const parent,
                           SearchModel* const searchModel,
                           SearchModificationHelper* const searchModificationHelper,
                           ItemFilterModel* const imageFilterModel,
                           QItemSelectionModel* const itemSelectionModel);
    ~GPSSearchView()                                override;

    void setActive(bool state);

    void changeAlbumFromHistory(SAlbum* const album);

    void doLoadState()                              override;
    void doSaveState()                              override;

    void setConfigGroup(const KConfigGroup& group)  override;

public Q_SLOTS:

    void slotRefreshMap();
    void slotClearImages();

private:

    bool checkName(QString& name);
    bool checkAlbum(const QString& name) const;

    void createNewGPSSearchAlbum(const QString& name);

private Q_SLOTS:

    void slotAlbumSelected(Album*);

    void slotSaveGPSSAlbum();
    void slotCheckNameEditGPSConditions();

    void slotRegionSelectionChanged();
    void slotRemoveCurrentFilter();
    void slotMapSoloItems(const QList<qlonglong>& idList);

    void showNonGeolocatedItems();

Q_SIGNALS:

    void signalMapSoloItems(const QList<qlonglong>& idList, const QString& id);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
