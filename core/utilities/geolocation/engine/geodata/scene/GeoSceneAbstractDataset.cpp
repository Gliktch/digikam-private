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

#include "GeoSceneAbstractDataset.h"

// C++ includes

#include <limits>

namespace Marble
{

// FIXME: Filters are a Dataset.

GeoSceneAbstractDataset::GeoSceneAbstractDataset(const QString& name)
    : m_name(name),
      m_fileFormat(),
      m_expire(std::numeric_limits<int>::max())
{
}

QString GeoSceneAbstractDataset::name() const
{
    return m_name;
}

QString GeoSceneAbstractDataset::fileFormat() const
{
    return m_fileFormat;
}

void GeoSceneAbstractDataset::setFileFormat(const QString& fileFormat)
{
    m_fileFormat = fileFormat;
}

int GeoSceneAbstractDataset::expire() const
{
    return m_expire;
}

void GeoSceneAbstractDataset::setExpire(int expire)
{
    m_expire = expire;
}

} // namespace Marble
