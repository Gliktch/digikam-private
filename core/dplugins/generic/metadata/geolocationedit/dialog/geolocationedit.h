/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : A tool to edit geolocation
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QModelIndex>
#include <QWidget>
#include <QUrl>

// Local includes

#include "dplugindialog.h"
#include "geoifacetypes.h"
#include "geomodelhelper.h"
#include "trackmanager.h"
#include "gpsitemcontainer.h"
#include "dinfointerface.h"
#include "mapwidget.h"
#include "gpsundocommand.h"

class QAbstractItemModel;

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class GeolocationEdit : public DPluginDialog
{
    Q_OBJECT

public:

    explicit GeolocationEdit(QWidget* const parent, DInfoInterface* const iface);
    ~GeolocationEdit()                  override;

    /**
     * Populate items in dialog list based on url. To be used in case of non database as with Showfoto.
     */
    void setImages(const QList<QUrl>& images);

    /**
     * Populate items in dialog list based dedicated GPSItemContainer instances filled with DB info
     */
    void setItems(const QList<GPSItemContainer*>& items);

Q_SIGNALS:

    void signalMetadataChangedForUrl(const QUrl&);

protected:

    void reject()                       override;
    void closeEvent(QCloseEvent* e)     override;
    bool eventFilter(QObject*, QEvent*) override;

private:

    void readSettings();
    void saveSettings();
    void saveChanges(const bool closeAfterwards);
    MapWidget* makeMapWidget(QWidget** const pvbox);
    void adjustMapLayout(const bool syncSettings);

private Q_SLOTS:

    void slotImageActivated(const QModelIndex& index);
    void slotSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void slotSetUIEnabled(const bool enabledState);
    void slotOkClicked();
    void slotApplyClicked();
    void slotFileChangesSaved(int beginIndex, int endIndex);
    void slotFileMetadataLoaded(int beginIndex, int endIndex);
    void slotProgressChanged(const int currentProgress);
    void slotProgressSetup(const int maxProgress, const QString& progressText);
    void slotGPSUndoCommand(GPSUndoCommand* undoCommand);
    void slotSortOptionTriggered(QAction* sortAction);
    void setCurrentTab(const int index);
    void slotProgressCancelButtonClicked();
    void slotCurrentTabChanged(int);
    void slotBookmarkVisibilityToggled();
    void slotLayoutChanged(int);
    void slotTrackListChanged(const Digikam::GeoCoordinates& coordinate);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGeolocationEditPlugin
