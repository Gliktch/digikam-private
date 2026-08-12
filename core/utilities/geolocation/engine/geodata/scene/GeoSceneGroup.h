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
#include <QVector>

// Local includes

#include "GeoDocument.h"

namespace Marble
{

class GeoSceneProperty;

/**
 * @short Group inside the settings of a GeoScene document.
 */
class GeoSceneGroup : public QObject,
    public GeoNode
{
    Q_OBJECT

public:

    explicit GeoSceneGroup(const QString& name);
    ~GeoSceneGroup() override;

    /**
     * @brief  Get the availability of a property in this group
     * @param  name  the property name
     * @param  available  availability of the property
     * @return @c true  the property was registered in this group
     *         @c false the property wasn't registered in this group
     */
    bool propertyAvailable(const QString& name, bool& available) const;

    /**
     * @brief  Set the value of a property in this group
     * @param  name  the property name
     * @param  value  the value of the property
     * @return @c true  the property was found and changed accordingly
     *         @c false the property couldn't be found in this group
     */
    bool setPropertyValue(const QString& name, bool value);

    /**
     * @brief  Get the value of a property in this group
     * @param  name  the property name
     * @param  value  the value of the property
     * @return @c true  the property was found and returned accordingly
     *         @c false the property couldn't be found in this group
     */
    bool propertyValue(const QString& name, bool& value) const;

    /**
     * @brief  Add a property to this setting group
     * @param  property  the new property
     */
    void addProperty(GeoSceneProperty*);
    const GeoSceneProperty* property(const QString& name) const;
    GeoSceneProperty* property(const QString& name);
    QVector<GeoSceneProperty*> properties();
    QVector<const GeoSceneProperty*> properties() const;

    QString name() const;

    const char* nodeType() const override;

Q_SIGNALS:

    void valueChanged(const QString&, bool);

private:

    Q_DISABLE_COPY(GeoSceneGroup)

private:

    /// The vector holding all the properties in this settings group.
    QVector<GeoSceneProperty*>  m_properties;

    QString                     m_name;
};

} // namespace Marble
