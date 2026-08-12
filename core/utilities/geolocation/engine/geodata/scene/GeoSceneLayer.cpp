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

#include "GeoSceneLayer.h"

// Local includes

#include "GeoSceneAbstractDataset.h"
#include "GeoSceneFilter.h"
#include "GeoSceneTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoSceneLayerPrivate
{
public:

    explicit GeoSceneLayerPrivate(const QString& name);
    ~GeoSceneLayerPrivate();

public:

    /// The vector holding all the data in the layer.
    /// (We want to preserve the order and don't care
    /// much about speed here), so we don't use a hash

    QVector<GeoSceneAbstractDataset*>   m_datasets;

    GeoSceneFilter*                     m_filter = nullptr;

    QString                             m_name;
    QString                             m_backend;
    QString                             m_role;

    bool                                m_tiled  = true;
};

GeoSceneLayerPrivate::GeoSceneLayerPrivate(const QString& name)
    : m_filter    (nullptr),
      m_name      (name),
      m_backend   (),
      m_role      (),
      m_tiled     (true)
{
}

GeoSceneLayerPrivate::~GeoSceneLayerPrivate()
{
    qDeleteAll(m_datasets);
}

GeoSceneLayer::GeoSceneLayer(const QString& name)
    : d(new GeoSceneLayerPrivate(name))
{
}

GeoSceneLayer::~GeoSceneLayer()
{
    delete d;
}

const char* GeoSceneLayer::nodeType() const
{
    return GeoSceneTypes::GeoSceneLayerType;
}

void GeoSceneLayer::addDataset(GeoSceneAbstractDataset* dataset)
{
    // Remove any dataset that has the same name

    QVector<GeoSceneAbstractDataset*>::iterator it = d->m_datasets.begin();

    while (it != d->m_datasets.end())
    {
        GeoSceneAbstractDataset* currentAbstractDataset = *it;

        if (currentAbstractDataset->name() == dataset->name())
        {
            delete currentAbstractDataset;
            d->m_datasets.erase(it);

            break;
        }
        else
        {
            ++it;
        }
    }

    if (dataset)
    {
        d->m_datasets.append(dataset);
    }
}

const GeoSceneAbstractDataset* GeoSceneLayer::dataset(const QString& name) const
{
    GeoSceneAbstractDataset* dataset                      = nullptr;

    QVector<GeoSceneAbstractDataset*>::const_iterator it  = d->m_datasets.constBegin();
    QVector<GeoSceneAbstractDataset*>::const_iterator end = d->m_datasets.constEnd();

    for ( ; it != end ; ++it)
    {
        if ((*it)->name() == name)
        {
            dataset = *it;
            break;
        }
    }

    return dataset;
}

// implement non-const method by means of const method,
// for details, see "Effective C++" (third edition)

GeoSceneAbstractDataset* GeoSceneLayer::dataset(const QString& name)
{
    return (
            const_cast<GeoSceneAbstractDataset*>
            (static_cast<GeoSceneLayer const*>(this)->dataset(name))
           );
}

const GeoSceneAbstractDataset* GeoSceneLayer::groundDataset() const
{
    if (d->m_datasets.isEmpty())
    {
        return nullptr;
    }

    return d->m_datasets.first();
}

// implement non-const method by means of const method,
// for details, see "Effective C++" (third edition)

GeoSceneAbstractDataset* GeoSceneLayer::groundDataset()
{
    return (
            const_cast<GeoSceneAbstractDataset*>
            (static_cast<GeoSceneLayer const*>(this)->groundDataset())
           );
}

QVector<GeoSceneAbstractDataset*> GeoSceneLayer::datasets() const
{
    return d->m_datasets;
}

QString GeoSceneLayer::name() const
{
    return d->m_name;
}

QString GeoSceneLayer::backend() const
{
    return d->m_backend;
}

void GeoSceneLayer::setBackend(const QString& backend)
{
    d->m_backend = backend;
}

bool GeoSceneLayer::isTiled() const
{
    return d->m_tiled;
}

void GeoSceneLayer::setTiled(bool tiled)
{
    d->m_tiled = tiled;
}

QString GeoSceneLayer::role() const
{
    return d->m_role;
}

void GeoSceneLayer::setRole(const QString& role)
{
    d->m_role = role;
}

const GeoSceneFilter* GeoSceneLayer::filter() const
{
    return d->m_filter;
}

GeoSceneFilter* GeoSceneLayer::filter()
{
    return d->m_filter;
}

void GeoSceneLayer::addFilter(GeoSceneFilter* filter)
{
    d->m_filter = filter;
}

void GeoSceneLayer::removeFilter(const GeoSceneFilter* filter)
{
    if (filter == d->m_filter)
    {
        d->m_filter = nullptr;
    }
}

} // namespace Marble
