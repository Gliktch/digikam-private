/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : A model to hold GPS information about items.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QItemSelection>

// Local includes

#include "gpsitemmodel.h"

namespace Digikam
{

class GPSItemSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit GPSItemSortProxyModel(GPSItemModel* const imageModel,
                                   QItemSelectionModel* const sourceSelectionModel);
    ~GPSItemSortProxyModel()                           override;

    QItemSelectionModel* mappedSelectionModel() const;

protected:

    bool lessThan(const QModelIndex& left,
                  const QModelIndex& right)      const override;

private:

    class Private;
    Private* const d = nullptr;
};

// ----------------------------------------------------------------------------------------------------------

class GPSLinkItemSelectionModelPrivate;

/**
 * Makes it possible to share a selection in multiple views which do not have the same source model
 *
 * Although Qt documentation, multiple views can share the same QItemSelectionModel,
 * the views then need to have the same source model.
 *
 * If there is a proxy model between the model and one of the views, or different proxy models in each, this class makes
 * it possible to share the selection between the views.
 */
class GPSLinkItemSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
    Q_PROPERTY(QItemSelectionModel* linkedItemSelectionModel
               READ linkedItemSelectionModel
               WRITE setLinkedItemSelectionModel
               NOTIFY linkedItemSelectionModelChanged)

public:

    GPSLinkItemSelectionModel(QAbstractItemModel* const targetModel,
                              QItemSelectionModel* const linkedItemSelectionModel,
                              QObject* const parent = nullptr);
    explicit GPSLinkItemSelectionModel(QObject* const parent = nullptr);
    ~GPSLinkItemSelectionModel()                                                              override;

    QItemSelectionModel* linkedItemSelectionModel() const;
    void setLinkedItemSelectionModel(QItemSelectionModel* const selectionModel);

    void select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)        override;
    void select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command) override;

Q_SIGNALS:

    void linkedItemSelectionModelChanged();

protected:

    GPSLinkItemSelectionModelPrivate* const d_ptr;

private:

    Q_DECLARE_PRIVATE(GPSLinkItemSelectionModel)
    Q_PRIVATE_SLOT(d_func(), void sourceSelectionChanged(const QItemSelection& selected,
                                                         const QItemSelection& deselected))
    Q_PRIVATE_SLOT(d_func(), void sourceCurrentChanged(const QModelIndex& current))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentChanged(const QModelIndex& current))
};

// ----------------------------------------------------------------------------------------------------------

class GPSModelIndexProxyMapperPrivate;

/**
 * This class facilitates easy mapping of indexes and selections through proxy models.
 *
 * In a complex system of proxy models there can be a need to map indexes and selections between them,
 * and sometimes to do so without knowledge of the path from one model to another.
 *
 * If there is a need to map indexes between proxy 2 and proxy 4, a GPSModelIndexProxyMapper can be created
 * to facilitate mapping of indexes between them.
 *
 *
 * Note that the aim is to achieve black box connections so that there is no need for application code to
 * know the structure of proxy models in the path between left and right and attempt to manually map them.
 *
 * The isConnected property indicates whether there is a
 * path from the left side to the right side.
 *
 */
class GPSModelIndexProxyMapper : public QObject
{
    Q_OBJECT

    /**
     * Indicates whether there is a chain that can be followed from leftModel to rightModel.
     *
     * This value can change if the sourceModel of an intermediate proxy is changed.
     */
    Q_PROPERTY(bool isConnected
               READ isConnected
               NOTIFY isConnectedChanged)

public:

    explicit GPSModelIndexProxyMapper(const QAbstractItemModel* const leftModel,
                                      const QAbstractItemModel* const rightModel,
                                      QObject* const parent = nullptr);
    ~GPSModelIndexProxyMapper() override;

    /**
     * Maps the @p index from the left model to the right model.
     */
    QModelIndex mapLeftToRight(const QModelIndex& index)                    const;

    /**
     * Maps the @p index from the right model to the left model.
     */
    QModelIndex mapRightToLeft(const QModelIndex& index)                    const;

    /**
     * Maps the @p selection from the left model to the right model.
     */
    QItemSelection mapSelectionLeftToRight(const QItemSelection& selection) const;

    /**
     * Maps the @p selection from the right model to the left model.
     */
    QItemSelection mapSelectionRightToLeft(const QItemSelection& selection) const;

    bool isConnected()                                                      const;

Q_SIGNALS:

    void isConnectedChanged();

private:

    Q_DECLARE_PRIVATE(GPSModelIndexProxyMapper)
    GPSModelIndexProxyMapperPrivate* const d_ptr;
};

} // namespace Digikam
