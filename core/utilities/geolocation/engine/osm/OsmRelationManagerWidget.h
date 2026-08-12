/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "MarbleGlobal.h"
#include "digikam_export.h"

class QTreeWidgetItem;

namespace Marble
{

class OsmRelationManagerWidgetPrivate;
class GeoDataPlacemark;
class OsmPlacemarkData;

enum Column
{
    Name = 0,
    Type = 1,
    Role = 2
};

/**
 * @brief The OsmRelationManagerWidget class is used to view and edit relations for any placemark
 * within the Annotate Plugin.
 * It can add the placemark to any relation the Annotate Plugin has loaded, or to newly created ones
 */
class DIGIKAM_EXPORT OsmRelationManagerWidget : public QWidget
{
    Q_OBJECT

public:

    explicit OsmRelationManagerWidget(GeoDataPlacemark* placemark,
                                      const QHash<qint64, OsmPlacemarkData>* relations,
                                      QWidget* parent = nullptr);
    ~OsmRelationManagerWidget() override;

public Q_SLOTS:

    /**
     * @brief addRelation adds the placemark to the relation specified in the action->text();
     * If the text is "New Relation", a new relation is added
     */
    void addRelation(QAction* action);
    void handleItemChange(QTreeWidgetItem* item, int column);
    void handleDoubleClick(QTreeWidgetItem* item, int column);
    void handleRelationContextMenuRequest(const QPoint& point);
    /**
     * @brief update updates the relations list and the suggested relations drop menu
     */
    void update();

Q_SIGNALS:

    void relationCreated(const OsmPlacemarkData& relationData);

private:

    friend class OsmRelationManagerWidgetPrivate;

    OsmRelationManagerWidgetPrivate* const d = nullptr;
};

} // namespace Marble
