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

#include <QString>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

/**
 * @short Contents used inside a layer.
 */
class DIGIKAM_EXPORT GeoSceneAbstractDataset : public GeoNode
{
public:

    ~GeoSceneAbstractDataset() override {};

    QString name() const;

    QString fileFormat() const;
    void setFileFormat(const QString& fileFormat);

    int expire() const;
    void setExpire(int expire);

protected:

    explicit GeoSceneAbstractDataset(const QString& name);

private:

    QString m_name;
    QString m_fileFormat;
    int     m_expire;
};

} // namespace Marble
