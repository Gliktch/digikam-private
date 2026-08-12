/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Robin Bussenot <bussenot dot robin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>

namespace MediaWiki
{

/**
 * @brief An image info.
 */
class Revision
{

public:

    /**
     * @brief Constructs a revision.
     */
    Revision();

    /**
     * @brief Destructs a revision.
     */
    ~Revision();

    /**
     * @brief Constructs a revision from an other revision.
     * @param other an other revision
     */
    Revision(const Revision& other);

    /**
     * @brief Assigning a revision from an other revision.
     * @param other an other revision
     */
    Revision& operator=(const Revision& other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const Revision& other)  const;

    /**
    * @brief Set the revision ID.
    * @param revisionId the revision ID
    **/
    void setRevisionId(int revisionId);

    /**
     * @brief Get the revision ID.
     * @return the revision ID
     */
    int revisionId()                        const;

    /**
     * @brief Set the parent ID.
     * @param parentId the parent ID
     */
    void setParentId(int parentId);

    /**
     * @brief Get the parent ID.
     * @return the parent ID
     */
    int parentId()                          const;

    /**
     * @brief Set the size of the revision text in bytes.
     * @param size the size of the revision text in bytes
     */
    void setSize(int size);

    /**
     * @brief Get the size of the revision text in bytes.
     * @return the size of the revision text in bytes
     */
    int size()                              const;

    /**
     * @brief Set true if the revision is minor.
     * @param minor true if the revision is minor
     */
    void setMinorRevision(bool minorRevision);

    /**
     * @brief Get true if the revision is minor.
     * @return true if the revision is minor
     */
    bool minorRevision()                    const;

    /**
     * @brief Get the date and time of the revision.
     * @return the date and time of the revision
     */
    QDateTime timestamp()                   const;

    /**
     * @brief Set the date and time of the revision
     * @param timestamp the date and time of the revision
     */
    void setTimestamp(const QDateTime& timestamp);

    /**
     * @brief Get the user who made the revision.
     * @return the user who made the revision
     */
    QString user()                          const;

    /**
     * @brief Set the user who made the revision.
     * @param user the user who made the revision
     */
    void setUser(const QString& user);

    /**
     * @brief The revision content.
     */
    QString content()                       const;

    /**
     * @brief Set the revision content.
     * @param content the revision content
     */
    void setContent(const QString& content);

    /**
     * @brief Get the edit comment.
     * @return the edit comment
     */
    QString comment()                       const;

    /**
     * @brief Set the edit comment.
     * @param comment the edit comment
     */
    void setComment(const QString& comment);

    /**
     * @brief Set the parse tree of the revision content.
     * @param parseTree the parse tree of the revision content
     */
    void setParseTree(const QString& parseTree);

    /**
     * @brief Get the parse tree of the revision content.
     * @return the parse tree of the revision content
     */
    QString parseTree()                     const;

    /**
     * @brief Set the rollback token.
     * @param rollback the rollback token
     */
    void setRollback(const QString& rollback);

    /**
     * @brief Get the rollback token.
     * @return the rollback token
     */
    QString rollback()                      const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace MediaWiki
