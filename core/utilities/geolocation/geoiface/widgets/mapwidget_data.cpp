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

void MapWidget::addUngroupedModel(GeoModelHelper* const modelHelper)
{
    s->ungroupedModels << modelHelper;

    /// @todo monitor all model signals!

    connect(modelHelper->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotUngroupedModelChanged()));

    connect(modelHelper->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotUngroupedModelChanged()));

    connect(modelHelper->model(), SIGNAL(modelReset()),
            this, SLOT(slotUngroupedModelChanged()));

    connect(modelHelper, SIGNAL(signalVisibilityChanged()),
            this, SLOT(slotUngroupedModelChanged()));

    if (modelHelper->selectionModel())
    {
        connect(modelHelper->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(slotUngroupedModelChanged()));
    }

    Q_EMIT signalUngroupedModelChanged(s->ungroupedModels.count() - 1);
}

void MapWidget::removeUngroupedModel(GeoModelHelper* const modelHelper)
{
    if (!modelHelper)
    {
        return;
    }

    const int modelIndex = s->ungroupedModels.indexOf(modelHelper);

    if (modelIndex < 0)
    {
        return;
    }

    /// @todo monitor all model signals!

    disconnect(modelHelper->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(slotUngroupedModelChanged()));

    disconnect(modelHelper->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(slotUngroupedModelChanged()));

    disconnect(modelHelper->model(), SIGNAL(modelReset()),
               this, SLOT(slotUngroupedModelChanged()));

    disconnect(modelHelper, SIGNAL(signalVisibilityChanged()),
               this, SLOT(slotUngroupedModelChanged()));

    if (modelHelper->selectionModel())
    {
        disconnect(modelHelper->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(slotUngroupedModelChanged()));
    }

    s->ungroupedModels.removeAt(modelIndex);

    // the indices changed, therefore send out notifications
    // sending out a signal with i=s->ungroupedModel.count()
    // will cause the backends to see that the last model is missing

    for (int i = modelIndex ; i <= s->ungroupedModels.count() ; ++i)
    {
        Q_EMIT signalUngroupedModelChanged(i);
    }
}

void MapWidget::setGroupedModel(AbstractMarkerTiler* const markerModel)
{
    s->markerModel = markerModel;

    if (s->markerModel)
    {
        s->markerModel->setActive(s->activeState);

        /// @todo this needs some buffering for the google maps backend

        connect(s->markerModel, SIGNAL(signalTilesOrSelectionChanged()),
                this, SLOT(slotRequestLazyReclustering()));

        if (d->currentBackend)
        {
            connect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(QVariant,QPixmap)),
                    d->currentBackend, SLOT(slotThumbnailAvailableForIndex(QVariant,QPixmap)));
        }
    }

    slotRequestLazyReclustering();
}

void MapWidget::setDragDropHandler(GeoDragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

void MapWidget::setTrackManager(TrackManager* const trackManager)
{
    s->trackManager = trackManager;

    // Some backends track the track manager activity even when not active
    // therefore they have to be notified.

    for (MapBackend* const backend : std::as_const(d->loadedBackends))
    {
        backend->slotTrackManagerChanged();
    }
}

void MapWidget::slotUngroupedModelChanged()
{
    // determine the index under which we handle this model

    QObject* const senderObject                 = sender();
    const QAbstractItemModel* const senderModel = qobject_cast<QAbstractItemModel*>(senderObject);

    if (senderModel)
    {
        for (int i = 0 ; i < s->ungroupedModels.count() ; ++i)
        {
            if (s->ungroupedModels.at(i)->model() == senderModel)
            {
                Q_EMIT signalUngroupedModelChanged(i);

                break;
            }
        }
        return;
    }

    const GeoModelHelper* const senderHelper = qobject_cast<GeoModelHelper*>(senderObject);

    if (senderHelper)
    {
        for (int i = 0 ; i < s->ungroupedModels.count() ; ++i)
        {
            if (s->ungroupedModels.at(i) == senderHelper)
            {
                Q_EMIT signalUngroupedModelChanged(i);

                break;
            }
        }
    }

    const QItemSelectionModel* const senderSelectionModel = qobject_cast<QItemSelectionModel*>(senderObject);

    if (senderSelectionModel)
    {
        for (int i = 0 ; i < s->ungroupedModels.count() ; ++i)
        {
            if (s->ungroupedModels.at(i)->selectionModel() == senderSelectionModel)
            {
                Q_EMIT signalUngroupedModelChanged(i);

                break;
            }
        }

        return;
    }
}

} // namespace Digikam
