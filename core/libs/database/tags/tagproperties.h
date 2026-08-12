/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of a tag in the database
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>
#include <QExplicitlySharedDataPointer>
#include <QMultiMap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT TagProperties
{
public:

    /**
     * This class provides a wrapper over the Database methods
     * to access the properties of a tag. It is meant to be a
     * short-lived object, it does not listen to external database changes.
     */
    TagProperties();

    /**
     * Access the properties of the given tag.
     */
    explicit TagProperties(int tagId);

    ~TagProperties();
    TagProperties(const TagProperties& other);
    TagProperties& operator=(const TagProperties& other);

    bool isNull()                                               const;

    int tagId()                                                 const;

    /**
     * Finds the tag for the given tag path or creates a new tag.
     * Then returns the tag properties for this tag.
     */
    static TagProperties getOrCreate(const QString& tagPath);

    /**
     * Returns true if the property is set.
     */
    bool hasProperty(const QString& key)                        const;

    /**
     * Returns true if the property is set, with exactly the given value.
     */
    bool hasProperty(const QString& key, const QString& value)  const;

    /**
     * Returns the value of the given property.
     * If the property is not set, a null string is returned.
     * But a null string is also returned if the property is set, but without a value.
     * Use hasProperty to check that case.
     */
    QString value(const QString& key)                           const;

    /// Returns all set property keys.
    QStringList propertyKeys()                                  const;

    /// Returns a map of all key->value pairs.
    QMultiMap<QString, QString> properties()                    const;

    /// Set the given property. Replaces all previous occurrences of this property.
    void setProperty(const QString& key, const QString& value);

    /**
     * Adds the given property. Does not change any previous occurrences of this property,
     * allowing multiple properties with the same key.
     */
    void addProperty(const QString& key, const QString& value);

    /// Remove the given property/value.
    void removeProperty(const QString& key, const QString& value);

    /// Remove all occurrences of the property.
    void removeProperties(const QString& key);

public:

    /// Declared as public cause it's used by TagPropertiesPrivSharedNull class.
    class TagPropertiesPriv;

private:

    QExplicitlySharedDataPointer<TagPropertiesPriv> d;
};

} // namespace Digikam
