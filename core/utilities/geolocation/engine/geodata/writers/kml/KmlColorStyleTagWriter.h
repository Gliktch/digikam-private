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

#include <QColor>

// Local includes

#include "GeoTagWriter.h"

namespace Marble
{

// No registration for this writer, ColorStyle is an abstract kml element
class KmlColorStyleTagWriter: public GeoTagWriter
{
public:

    explicit KmlColorStyleTagWriter(const QString& elementName);

    bool write(const GeoNode* node, GeoWriter& writer) const override;

    static QString formatColor(const QColor& color);

protected:

    virtual bool writeMid(const GeoNode* node, GeoWriter& writer) const = 0;

    /** Returns true iff all properties have a default value */
    virtual bool isEmpty(const GeoNode* node) const = 0;

    /** Returns the default color **/
    virtual QColor defaultColor() const;

private:

    QString const m_elementName;
};

} // namespace Marble
