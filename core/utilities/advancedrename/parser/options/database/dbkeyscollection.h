/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a db option key
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>

// Local includes

#include "parsesettings.h"

namespace Digikam
{

typedef QMap<QString, QString> DbKeyIdsMap;

/**
 * @brief A class for managing / grouping database keys.
 *
 * This class manages database keys and provides methods to get the
 * appropriate value from the database.
 */
class DbKeysCollection
{
public:

    /**
     * Default constructor.
     *
     * @param n collection name
     */
    explicit DbKeysCollection(const QString& n);
    virtual ~DbKeysCollection() = default;

    /**
     * Get a value from the database.
     * @param key           the key representing the value in the database
     * @param settings      the %ParseSettings object holding all relevant information
     *                      about the image.
     * @return  the value of the given database key
     */
    QString getValue(const QString& key, ParseSettings& settings);

    /**
     * Get all IDs associated with this key collection.
     * @return a map of all associated ids and their description
     */
    DbKeyIdsMap ids() const;

    /**
     * Get the name of the %DbKeysCollection
     * @return the name of the collection
     */
    QString collectionName() const;

protected:

    /**
     * Abstract method for retrieving the value from the database for the given key.
     *
     * This method has to be implemented by all child classes. It is called by the
     * getValue() method.
     *
     * @param key           the key representing the value in the database
     * @param settings      the %ParseSettings object holding all relevant information
     *                      about the image.
     *
     * @return  the value of the given database key
     * @see DbKeysCollection::getValue()
     */
    virtual QString getDbValue(const QString& key, ParseSettings& settings) = 0;

    /**
     * Add an ID to the key collection.
     *
     * @param id            the id of the database key
     * @param description   a short description of the database key
     */
    void addId(const QString& id, const QString& description);

private:

    DbKeyIdsMap idsMap;
    QString     name;

private:

    DbKeysCollection(const DbKeysCollection&)            = delete;
    DbKeysCollection& operator=(const DbKeysCollection&) = delete;
};

} // namespace Digikam
