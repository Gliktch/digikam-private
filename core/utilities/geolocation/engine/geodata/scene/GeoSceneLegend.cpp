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

#include "GeoSceneLegend.h"

// Local includes

#include "GeoSceneTypes.h"
#include "GeoSceneSection.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoSceneLegendPrivate
{
public:

    ~GeoSceneLegendPrivate()
    {
        qDeleteAll(m_sections);
    }

    /// The vector holding all the sections in the legend.
    /// (We want to preserve the order and don't care
    /// much about speed here), so we don't use a hash

    QVector<const GeoSceneSection*> m_sections;
};

GeoSceneLegend::GeoSceneLegend()
    : d(new GeoSceneLegendPrivate)
{
}

GeoSceneLegend::~GeoSceneLegend()
{
    delete d;
}

const char* GeoSceneLegend::nodeType() const
{
    return GeoSceneTypes::GeoSceneLegendType;
}

void GeoSceneLegend::addSection(const GeoSceneSection* section)
{
    // Remove any section that has the same name

    QVector<const GeoSceneSection*>::iterator it = d->m_sections.begin();

    while (it != d->m_sections.end())
    {
        const GeoSceneSection* currentSection = *it;

        if (currentSection->name() == section->name())
        {
            delete currentSection;
            d->m_sections.erase(it);
            break;
        }

        else
        {
            ++it;
        }
    }

    if (section)
    {
        d->m_sections.append(section);
    }
}

QVector<const GeoSceneSection*> GeoSceneLegend::sections() const
{
    return d->m_sections;
}

} // namespace Marble
