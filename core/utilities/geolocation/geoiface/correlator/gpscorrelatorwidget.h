/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-26
 * Description : A widget to configure the GPS correlation
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014      by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QWidget>
#include <QItemSelectionModel>

// Local includes

#include "track_correlator.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class GPSItemModel;
class GPSUndoCommand;

class DIGIKAM_EXPORT GPSCorrelatorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit GPSCorrelatorWidget(QWidget* const parent,
                                 GPSItemModel* const imageModel,
                                 QItemSelectionModel* const selectionModel,
                                 TrackManager* const trackManager);
    ~GPSCorrelatorWidget() override;

    void setUIEnabledExternal(const bool state);
    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    QList<GeoCoordinates::List> getTrackCoordinates()           const;
    bool getShowTracksOnMap()                                   const;

protected:

    void setUIEnabledInternal(const bool state);

Q_SIGNALS:

    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabledWithCancel(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void signalProgressSetup(const int maxProgress, const QString& progressText);
    void signalProgressChanged(const int currentProgress);
    void signalUndoCommand(GPSUndoCommand* undoCommand);
    void signalAllTrackFilesReady();
    void signalTrackListChanged(const Digikam::GeoCoordinates& coordinate);

public Q_SLOTS:

    void slotCancelCorrelation();

private Q_SLOTS:

    void updateUIState();
    void slotLoadTrackFiles();
    void slotAllTrackFilesReady();
    void slotCorrelate();
    void slotItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void slotAllItemsCorrelated();
    void slotCorrelationCanceled();
    void slotShowTracksStateChanged(int state);
    void slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
