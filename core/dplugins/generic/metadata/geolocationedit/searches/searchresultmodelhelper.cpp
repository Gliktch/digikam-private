/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchresultmodelhelper.h"

// Qt includes

#include <QContextMenuEvent>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QStandardPaths>
#include <QLineEdit>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "gpscommon.h"

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class Q_DECL_HIDDEN SearchResultModelHelper::Private
{
public:

    Private() = default;

    SearchResultModel*   model          = nullptr;
    QItemSelectionModel* selectionModel = nullptr;
    GPSItemModel*        imageModel     = nullptr;
    bool                 visible        = true;
};

SearchResultModelHelper::SearchResultModelHelper(SearchResultModel* const resultModel,
                                                 QItemSelectionModel* const selectionModel,
                                                 GPSItemModel* const imageModel,
                                                 QObject* const parent)
    : GeoModelHelper(parent),
      d             (new Private)
{
    d->model          = resultModel;
    d->selectionModel = selectionModel;
    d->imageModel     = imageModel;
}

SearchResultModelHelper::~SearchResultModelHelper()
{
    delete d;
}

QAbstractItemModel* SearchResultModelHelper::model() const
{
    return d->model;
}

QItemSelectionModel* SearchResultModelHelper::selectionModel() const
{
    return d->selectionModel;
}

bool SearchResultModelHelper::itemCoordinates(const QModelIndex& index,
                                              GeoCoordinates* const coordinates) const
{
    const SearchResultModel::SearchResultItem item = d->model->resultItem(index);
    *coordinates                                   = item.result.coordinates;

    return true;
}

void SearchResultModelHelper::setVisibility(const bool state)
{
    d->visible = state;

    Q_EMIT signalVisibilityChanged();
}

bool SearchResultModelHelper::itemIcon(const QModelIndex& index,
                                       QPoint* const offset,
                                       QSize* const size,
                                       QPixmap* const pixmap,
                                       QUrl* const url) const
{
    return d->model->getMarkerIcon(index, offset, size, pixmap, url);
}

GeoModelHelper::PropertyFlags SearchResultModelHelper::modelFlags() const
{
    return (FlagSnaps | (d->visible ? FlagVisible:FlagNull));
}

GeoModelHelper::PropertyFlags SearchResultModelHelper::itemFlags(const QModelIndex& /*index*/) const
{
    return (FlagVisible | FlagSnaps);
}

void SearchResultModelHelper::snapItemsTo(const QModelIndex& targetIndex,
                                          const QList<QModelIndex>& snappedIndices)
{
    GPSUndoCommand* const undoCommand              = new GPSUndoCommand();
    SearchResultModel::SearchResultItem targetItem = d->model->resultItem(targetIndex);
    const GeoCoordinates& targetCoordinates        = targetItem.result.coordinates;

    for (int i = 0 ; i < snappedIndices.count() ; ++i)
    {
        const QPersistentModelIndex itemIndex = snappedIndices.at(i);
        GPSItemContainer* const item          = d->imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(item);

        GPSDataContainer newData;
        newData.setCoordinates(targetCoordinates);
        item->setGPSData(newData);

        undoInfo.readNewDataFromItem(item);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(i18np("1 image snapped to '%2'",
                               "%1 images snapped to '%2'", snappedIndices.count(), targetItem.result.name));

    Q_EMIT signalUndoCommand(undoCommand);
}

} // namespace DigikamGenericGeolocationEditPlugin

#include "moc_searchresultmodelhelper.cpp"
