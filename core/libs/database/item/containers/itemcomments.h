/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Access to comments of an item in the database
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Patrick Spendrin <ps_ml at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>
#include <QSharedDataPointer>
#include <QSharedData>
#include <QSet>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"
#include "captionvalues.h"
#include "coredbaccess.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemComments
{
public:

    /**
     * @brief The ItemComments class shall provide short-lived objects that provide read/write access
     * to the comments stored in the database. It is a mere wrapper around the less
     * convenient access methods in CoreDB.
     * Database results are cached, but the object will not listen to database changes from other places.
     *
     * Changes are applied to the database only after calling apply(), which you can call any time
     * and which will in any case be called from the destructor.
     */

    enum LanguageChoiceBehavior
    {
        /**
         *  Return only a comment if the language code
         *  (at least the language code, the country part may differ)
         *  is identical. Else returns a null QString.
         */
        ReturnMatchingLanguageOnly,

        /**
         * If no matching language as above is found, return the default language.
         */
        ReturnMatchingOrDefaultLanguage,

        /**
         * If no matching or default language is found, return the first comment.
         * Returns a null string only if no comment is available.
         */
        ReturnMatchingDefaultOrFirstLanguage
    };

    enum UniqueBehavior
    {
        /**
         * @brief Allow only one comment per language. Default setting.
         */
        UniquePerLanguage,

        /**
         * @brief Allow multiple comments per language, each with a different author
         */
        UniquePerLanguageAndAuthor
    };

public:

    /**
     * @brief Create a null ItemComments object
     */
    ItemComments();

    /**
     * @brief Create a ItemComments object for the image with the specified id.
     */
    explicit ItemComments(qlonglong imageid);

    /**
     * @brief Create a ItemComments object for the image with the specified id.
     * The existing CoreDbAccess object will be used to access the database.
     */
    ItemComments(const CoreDbAccess& access, qlonglong imageid);

    ItemComments(const ItemComments& other);
    ~ItemComments();

    ItemComments& operator=(const ItemComments& other);

    bool isNull()                                                                                               const;

    /**
     * @brief Changes the behavior to unique comments per language, see the enum above for possible
     * values.
     * Default value is UniquePerLanguage.
     * @note This is _not_ a property of the database, but only of this single ItemComments object,
     */
    void setUniqueBehavior(UniqueBehavior behavior);

    /**
     * @brief This methods presents one of the comment strings of the available comment
     * as the default value, when you just want to have one string.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString defaultComment(DatabaseComment::Type type = DatabaseComment::Comment)                               const;

    QString defaultComment(int* const index, Digikam::DatabaseComment::Type type = DatabaseComment::Comment)    const;

    /**
     * @return a comment for the specified language.
     * Matching behavior can be specified.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString commentForLanguage(const QString& languageCode, int* const index = nullptr,
                               LanguageChoiceBehavior behavior = ReturnMatchingDefaultOrFirstLanguage)          const;

    /**
     * @return the number of comments available.
     */
    int numberOfComments()                                                                                      const;

    /**
     * @brief Access individual properties. Please ensure that the specified index is a valid index
     */
    DatabaseComment::Type type(int index)                                                                       const;

    /**
     * @return the RFC 3066 notation, or "x-default"
     */
    QString language(int index)                                                                                 const;

    QString author(int index)                                                                                   const;
    QDateTime date(int index)                                                                                   const;
    QString comment(int index)                                                                                  const;

    /**
     * @brief Add a new comment to the list of normal image comments, specified with language and author.
     * Checking for unique comments is done as set by setUniqueBehavior.
     * If you pass a null string as language, it will be translated to the language code designating
     * the default language ("x-default").
     * If you just want to change the one comment of the image, call addComment(myComment);
     */
    void addComment(const QString& comment,
                    const QString& language = QString(),
                    const QString& author = QString(),
                    const QDateTime& date = QDateTime(),
                    DatabaseComment::Type type = DatabaseComment::Comment);

    /**
     * @brief Convenience method to add a comment of type Headline. Calls addComment, see above for more info.
     */
    void addHeadline(const QString& headline,
                     const QString& language = QString(),
                     const QString& author = QString(),
                     const QDateTime& date = QDateTime());

    /**
     * @brief Convenience method to add a comment of type Headline. Calls addComment, see above for more info.
     */
    void addTitle(const QString& title,
                  const QString& language = QString(),
                  const QString& author = QString(),
                  const QDateTime& date = QDateTime());

    /**
     * @brief Replaces all existing comments with the given set of comments and associated language.
     * Optionally date and author can be specified in CaptionsMap container.
     */
    void replaceComments(const CaptionsMap& comments,
                         DatabaseComment::Type type = DatabaseComment::Comment);

    /**
     * @brief Remove the entry referred to by index.
     */
    void remove(int index);

    /**
     * @brief Remove all entries of the given type
     */
    void removeAll(DatabaseComment::Type type);

    /**
     * @brief Convenience method: remove all entries of type Comment
     */
    void removeAllComments();

    /**
     * @brief Remove all entries of all types: Comments, Headlines, Titles
     */
    void removeAll();

    /**
     * @brief Access individual properties.
     * Please ensure that the specified index is a valid index
     */
    void changeComment(int index, const QString& comment);
    void changeLanguage(int index, const QString& language);
    void changeAuthor(int index, const QString& author);
    void changeDate(int index, const QDateTime& date);
    void changeType(int index, DatabaseComment::Type type);

    /**
     * @brief Apply all changes.
     * Also called in destructor, so you typically do not need to call this.
     */
    void apply();
    void apply(const CoreDbAccess& access);

    /**
     * @return all entries of the given type in a CaptionsMap container.
     */
    CaptionsMap toCaptionsMap(DatabaseComment::Type = DatabaseComment::Comment)                                 const;

    /**
     * @brief Replaces all entries in this object with all entries from source.
     */
    void replaceFrom(const ItemComments& source);

protected:

    void addCommentDirectly(const QString& comment,
                            const QString& language,
                            const QString& author,
                            DatabaseComment::Type type,
                            const QDateTime& date);
public:

    class Private;

protected:

    QSharedDataPointer<Private> d;
};

} // namespace Digikam
