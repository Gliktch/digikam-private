/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Remi Benoit <r3m1 dot benoit at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>

namespace MediaWiki
{

/**
 * @brief A user group.
 */
class UserGroup
{

public:

    /**
     * Constructs a user group.
     */
    UserGroup();

    /**
     * @brief Constructs a user group from an other user group.
     * @param other an other user group
     */
    UserGroup(const UserGroup& other);

    /**
     * @brief Destructs a user group.
     */
    ~UserGroup();

    /**
     * @brief Assigning a user group from an other user group.
     * @param other an other user group
     */
    UserGroup& operator=(const UserGroup& other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const UserGroup& other) const;

    /**
     * @brief Returns the name of the user group.
     * @return the name of the user group
     */
    QString name()                          const;

    /**
     * @brief Set the name of the user group.
     * @param name the name of the user group
     */
    void setName(const QString& name);

    /**
     * @brief Returns rights of the user group.
     * @return rights of the user group
     */
    const QList<QString>& rights()          const;

    /**
     * @brief Returns rights of the user group.
     * @return rights of the user group
     */
    QList<QString>& rights();

    /**
     * @brief Set rights of the user group.
     * @param rights the rights of the user group
     */
     void setRights(const QList<QString>& rights);

    /**
     * @brief Returns the numbers of users in the user group.
     * @return the numbers of users in the user group
     */
    qint64 number()                         const;

    /**
     * @brief Set the number of users in the user group.
     * @param number the number of users in the user group
     */
    void setNumber(qint64 number);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace MediaWiki
