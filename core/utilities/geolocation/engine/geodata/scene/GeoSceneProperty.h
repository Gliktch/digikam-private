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

#include <QObject>
#include <QString>

// Local includes

#include "GeoDocument.h"
#include "digikam_export.h"

namespace Marble
{

/**
 * @short Settings property within a GeoScene document.
 */
class DIGIKAM_EXPORT GeoSceneProperty : public QObject,
    public GeoNode
{
    Q_OBJECT

public:

    explicit GeoSceneProperty(const QString& name);
    const char* nodeType() const override;

    QString name() const;
    bool available() const;
    void setAvailable(bool available);
    bool defaultValue() const;
    void setDefaultValue(bool defaultValue);
    bool value() const;
    void setValue(bool value);

Q_SIGNALS:

    void valueChanged(const QString&, bool);

private:

    QString  m_name;
    bool     m_available;
    bool     m_defaultValue;
    bool     m_value;
};

} // namespace Marble
