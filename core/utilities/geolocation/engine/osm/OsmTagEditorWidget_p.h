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

#include <QPair>

// Local includes

#include "ui_OsmTagEditorWidget.h"

namespace Marble
{

class GeoDataPlacemark;
class OsmPlacemarkData;
class OsmTagEditorWidget;

class Q_DECL_HIDDEN OsmTagEditorWidgetPrivate : public Ui::OsmTagEditorWidgetPrivate
{
public:

    typedef QPair<QString, QString> OsmTag;
    OsmTagEditorWidgetPrivate();
    ~OsmTagEditorWidgetPrivate();
    void populatePresetTagsList();
    void populateCurrentTagsList();

private:

    friend class OsmTagEditorWidget;
    static QTreeWidgetItem* tagWidgetItem(const OsmTag& tag);

    /**
     * @brief recommendedTags builds a list of tags that are recommended by osm standards
     * taking the placemark's type and osmData in consideration.
     */
    QList<OsmTag> recommendedTags() const;

    /**
     * @brief generateTagFilter this is where the filter criteria for the recommended tags list
     * are set.
     */
    QStringList generateTagFilter() const;

    /**
     * @brief containsAny convenience function that determines whether the placemark contains
     * any of the given parameter tags
     */
    static bool containsAny(const OsmPlacemarkData& osmData, const QStringList& tags);

    /**
     * @brief addPattern convenience function that takes a list of OsmTags
     * and adds their keys to the filter, if the placemark doesn't already have them
     */
    static void addPattern(QStringList& filter, const OsmPlacemarkData& osmData, const QStringList& pattern);

    /**
     * @brief returns a list of useful, popular osm tags that currently don't have a visual category associated with them
     */
    static QVector<OsmTag> createAdditionalOsmTags();

private:

    GeoDataPlacemark*    m_placemark = nullptr;

    static const QString m_customTagAdderText;
};

} // namespace Marble
