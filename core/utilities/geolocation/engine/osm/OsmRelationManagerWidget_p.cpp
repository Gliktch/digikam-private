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

#include "OsmRelationManagerWidget_p.h"
#include "OsmRelationManagerWidget.h"

// Qt includes

#include <QTreeWidget>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "OsmPlacemarkData.h"
#include "digikam_debug.h"

namespace Marble
{

OsmRelationManagerWidgetPrivate::OsmRelationManagerWidgetPrivate()
{
    // nothing to do
}

OsmRelationManagerWidgetPrivate::~OsmRelationManagerWidgetPrivate()
{
    // nothing to do
}

void OsmRelationManagerWidgetPrivate::populateRelationsList()
{
    m_currentRelations->clear();

    // This shouldn't happen
    if (!m_allRelations)
    {
        return;
    }

    if (m_placemark->hasOsmData())
    {
        const OsmPlacemarkData& osmData = m_placemark->osmData();
        auto it = osmData.relationReferencesBegin();
        const auto end = osmData.relationReferencesEnd();

        for (; it != end; ++it)
        {

            if (!m_allRelations->contains(it.key().id))
            {
                qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("Relation %1 is not loaded in the Annotate Plugin").arg(it.key().id);
                continue;
            }

            const OsmPlacemarkData& relationData = m_allRelations->value(it.key().id);

            QTreeWidgetItem* newItem = new QTreeWidgetItem();
            QString name = relationData.tagValue(QStringLiteral("name"));
            QString type = relationData.tagValue(QStringLiteral("type"));
            QString role = it.value();
            newItem->setText(Column::Name, name);
            newItem->setText(Column::Type, type);
            newItem->setText(Column::Role, role);
            newItem->setData(Column::Name, Qt::UserRole, relationData.id());
            m_currentRelations->addTopLevelItem(newItem);

        }
    }
}

void OsmRelationManagerWidgetPrivate::populateDropMenu()
{
    m_relationDropMenu->clear();

    m_addRelation->setIcon(QIcon::fromTheme(QLatin1String("list-add")));

    // The new relation adder

    m_relationDropMenu->addAction(i18n("New Relation"));
    m_relationDropMenu->addSeparator();

    // This shouldn't happen

    Q_ASSERT(m_allRelations);

    // Suggesting existing relations

    const auto values = m_allRelations->values();

    for (const OsmPlacemarkData& relationData : values)
    {
        const QString relationText = relationData.tagValue(QString::fromUtf8("name")) + QLatin1String(" (") + relationData.tagValue(QString::fromUtf8("type")) + QLatin1Char(')');

        // Don't suggest relations the placemark is already part of

        if (m_placemark->hasOsmData() && m_placemark->osmData().containsRelation(relationData.id()))
        {
            continue;
        }

        QAction* newAction = new QAction(m_relationDropMenu);
        newAction->setText(relationText);
        newAction->setData(relationData.id());
        m_relationDropMenu->addAction(newAction);
    }
}

} // namespace Marble
