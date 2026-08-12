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

class OsmTagEditorWidgetPrivate;
class GeoDataPlacemark;
class GeoDataFeature;
class OsmPlacemarkData;

class DIGIKAM_EXPORT OsmTagEditorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit OsmTagEditorWidget(GeoDataPlacemark* placemark, QWidget* parent = nullptr);
    ~OsmTagEditorWidget() override;

    /**
     * @brief returns the current tag set for the placemark
     */
    OsmPlacemarkData placemarkData() const;

public Q_SLOTS:

    void update();
    void addSelectedTag();
    void removeSelectedTag();
    void handleDoubleClick(QTreeWidgetItem* item, int column);
    void handleItemChanged(QTreeWidgetItem* item, int column);

Q_SIGNALS:

    void placemarkChanged(GeoDataFeature*);

private:

    friend class OsmTagEditorWidgetPrivate;

    OsmTagEditorWidgetPrivate* const d = nullptr;
};

} // namespace Marble

