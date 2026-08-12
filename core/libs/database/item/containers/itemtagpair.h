/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of an Item / Tag pair, i.e., a tag associated to an item
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

class ItemInfo;
class ItemTagPairPriv;

class DIGIKAM_DATABASE_EXPORT ItemTagPair
{
public:

    /**
     * @brief This class provides a wrapper over the Database methods
     * to access the properties of tag / image association. It is meant to be a
     * short-lived object, it does not listen to external database changes.
     */

    /**
     * @brief Creates a null pair.
     */
    ItemTagPair();

    /**
     * @brief Access the properties of the given image - tag pair
     */
    ItemTagPair(qlonglong imageId, int tagId);
    ItemTagPair(const ItemInfo& info, int tagId);

    ~ItemTagPair();

    ItemTagPair(const ItemTagPair& other);
    ItemTagPair& operator=(const ItemTagPair& other);

    bool isNull() const;

    /**
     * @return all pairs for the given image for which entries exist.
     * This list of tags may not be identical to the tags assigned to the image.
     */
    static QList<ItemTagPair> availablePairs(qlonglong imageId);
    static QList<ItemTagPair> availablePairs(const ItemInfo& info);

    qlonglong imageId() const;
    int tagId() const;

    /**
     * @return if the tag is assigned to the image
     */
    bool isAssigned() const;

    /**
     * @brief Assigns the tag to the image
     */
    void assignTag();

    /**
     * @brief Removes the tag from the image
     */
    void unAssignTag();

    /// @return true if the property is set
    bool hasProperty(const QString& key) const;
    /// @return true if any of the properties is set
    bool hasAnyProperty(const QStringList& keys) const;
    /// @return true of the given property and value is set
    bool hasValue(const QString& key, const QString& value) const;
    /// @return the value of the given property, or a null string if not set
    QString value(const QString& key) const;
    /// @return value() concatenated for all given keys
    QStringList allValues(const QStringList& keys) const;
    /// @return a list of values with the given property
    QStringList values(const QString& key) const;
    /// @return all set property keys
    QStringList propertyKeys() const;
    /// @return a map of all key->value pairs
    QMultiMap<QString, QString> properties() const;

    /// @brief Set the given property. Replaces all previous occurrences of this property.
    void setProperty(const QString& key, const QString& value);

    /**
     * @brief Adds the given property. Does not change any previous occurrences of this property,
     * allowing multiple properties with the same key.
     * (duplicates of same key _and_ value are not added, though)
     */
    void addProperty(const QString& key, const QString& value);

    /// @brief Remove all occurrences of the property
    void removeProperty(const QString& key, const QString& value);
    /// @brief Remove all occurrences of the property
    void removeProperties(const QString& key);
    /// @brief Removes all properties
    void clearProperties();

private:

    QExplicitlySharedDataPointer<ItemTagPairPriv> d;
};

} // namespace
