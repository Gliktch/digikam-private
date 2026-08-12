/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : metadata Settings Container.
 *
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>
#include <QString>
#include <QMap>
#include <QDebug>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

/**
 * @brief The NamespaceEntry class provide a simple container
 *        for dmetadata namespaces variables, such
 *        as names, what types of data expects and extra
 *        xml tags
 */
class DIGIKAM_EXPORT NamespaceEntry
{

public:

    enum NsSubspace
    {
        EXIF = 0,
        IPTC = 1,
        XMP  = 2
    };

    enum TagType
    {
        TAG     = 0,
        TAGPATH = 1
    };

    enum SpecialOptions
    {
        NO_OPTS             = 0,
        COMMENT_ALTLANG     = 1,
        COMMENT_ATLLANGLIST = 2,
        COMMENT_XMP         = 3,
        COMMENT_JPEG        = 4,
        TAG_XMPBAG          = 5,
        TAG_XMPSEQ          = 6,
        TAG_ACDSEE          = 7,
        TAG_ALTLANG         = 8
    };

    enum NamespaceType
    {
        TAGS       = 0,
        TITLE      = 1,
        RATING     = 2,
        COMMENT    = 3,
        PICKLABEL  = 4,
        COLORLABEL = 5
    };

public:

    NamespaceEntry() = default;

    NamespaceEntry(const NamespaceEntry& other)
      : nsType          (other.nsType),
        subspace        (other.subspace),
        isDefault       (other.isDefault),
        isDisabled      (other.isDisabled),
        index           (other.index),
        namespaceName   (other.namespaceName),
        alternativeName (other.alternativeName),
        tagPaths        (other.tagPaths),
        separator       (other.separator),
        convertRatio    (QList<int>(other.convertRatio)),
        specialOpts     (other.specialOpts),
        secondNameOpts  (other.secondNameOpts)
    {
    }

    ~NamespaceEntry() = default;

public:

    static QString DM_TAG_CONTAINER();
    static QString DM_TITLE_CONTAINER();
    static QString DM_RATING_CONTAINER();
    static QString DM_COMMENT_CONTAINER();
    static QString DM_PICKLABEL_CONTAINER();
    static QString DM_COLORLABEL_CONTAINER();

public:

    NamespaceType  nsType           = TAGS;
    NsSubspace     subspace         = XMP;
    bool           isDefault        = true;
    bool           isDisabled       = false;
    int            index            = -1;

    /**
     * Tag Options
     */
    QString        namespaceName;
    QString        alternativeName;
    TagType        tagPaths         = TAGPATH;
    QString        separator;

    /**
     * Rating Options
     */
    QList<int>     convertRatio;

    SpecialOptions specialOpts      = NO_OPTS;
    SpecialOptions secondNameOpts   = NO_OPTS;
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const NamespaceEntry& inf);

/**
 * The class DMetadataSettingsContainer is designed to dynamically add namespaces.
 */
class DIGIKAM_EXPORT DMetadataSettingsContainer
{
public:

    DMetadataSettingsContainer();
    DMetadataSettingsContainer(const DMetadataSettingsContainer& other);
    ~DMetadataSettingsContainer();

    DMetadataSettingsContainer& operator=(const DMetadataSettingsContainer& other);

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group)                                                         const;

    /**
     * @brief defaultValues - default namespaces used by digiKam
     */
    void defaultValues();

    bool unifyReadWrite()                                                                           const;
    void setUnifyReadWrite(bool b);

    bool readingAllTags()                                                                           const;
    void setReadingAllTags(bool b);

    void addMapping(const QString& key);

    QList<NamespaceEntry>& getReadMapping(const QString& key)                                       const;

    QList<NamespaceEntry>& getWriteMapping(const QString& key)                                      const;

    QList<QString> mappingKeys()                                                                    const;

    QString translateMappingKey(const QString& key)                                                 const;

private:

    void defaultTagValues();
    void defaultTitleValues();
    void defaultRatingValues();
    void defaultCommentValues();
    void defaultPickLabelValues();
    void defaultColorLabelValues();
    void readOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container);
    void writeOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container)  const;

private:

    class Private;
    Private* const d = nullptr;
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const DMetadataSettingsContainer& inf);

} // namespace Digikam
