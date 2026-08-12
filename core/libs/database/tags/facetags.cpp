/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-08
 * Description : Accessing face tags
 *
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facetags.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredbconstants.h"
#include "tagscache.h"
#include "tagregion.h"
#include "tagproperties.h"

namespace Digikam
{

// --- FaceIfacePriv ---

class Q_DECL_HIDDEN FaceTagsHelper
{
public:

    static QString tagPath(const QString& name, int parentId);
    static void    makeFaceTag(int tagId, const QString& fullName);
    static int     findFirstTagWithProperty(const QString& property, const QString& value = QString());
    static int     tagForName(const QString& name, int tagId, int parentId,
                              const QString& givenFullName, bool convert, bool create);
};

// --- Private methods ---

int FaceTagsHelper::findFirstTagWithProperty(const QString& property, const QString& value)
{
    QList<int> candidates = TagsCache::instance()->tagsWithProperty(property, value);

    if (!candidates.isEmpty())
    {
        return candidates.first();
    }

    return 0;
}

QString FaceTagsHelper::tagPath(const QString& name, int parentId)
{
    QString faceParentTagPath = TagsCache::instance()->tagPath(parentId,
                                                               TagsCache::NoLeadingSlash);

    if (
        faceParentTagPath.contains(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|"
                                                                    "/_Digikam_root_tag_|"
                                                                    "_Digikam_root_tag_)")))
       )
    {
        return  (QLatin1Char('/') + name);
    }

    return (faceParentTagPath + QLatin1Char('/') + name);
}

void FaceTagsHelper::makeFaceTag(int tagId, const QString& fullName)
{
    QString faceEngineName  = fullName;

/*
    // Find a unique FacesEngineId.

    for (int i = 0 ; d->findFirstTagWithProperty(TagPropertyName::FacesEngineId(), FacesEngineId) ; ++i)
    {
        FacesEngineId = fullName + QString::fromUtf8(" (%1)").arg(i);
    }
*/

    TagProperties props(tagId);
    props.setProperty(TagPropertyName::person(),         fullName);
    props.setProperty(TagPropertyName::faceEngineName(), faceEngineName);
}

int FaceTagsHelper::tagForName(const QString& name, int tagId, int parentId,
                               const QString& givenFullName,
                               bool convert, bool create)
{
    if (name.isEmpty() && givenFullName.isEmpty() && !tagId)
    {
        return FaceTags::unknownPersonTagId();
    }

    QString fullName = (givenFullName.isNull() ? name : givenFullName);

    if (tagId)
    {
        if      (FaceTags::isPerson(tagId))
        {
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "Proposed tag is already a person";
*/
            return tagId;
        }
        else if (convert)
        {
            if (fullName.isNull())
            {
                fullName = TagsCache::instance()->tagName(tagId);
            }

            qCDebug(DIGIKAM_DATABASE_LOG) << "Converting proposed tag to person, full name" << fullName;
            makeFaceTag(tagId, fullName);

            return tagId;
        }

        return 0;
    }

    // First attempt: Find by full name in "person" attribute.

    QList<int> candidates = TagsCache::instance()->tagsWithProperty(TagPropertyName::person(), fullName);

    for (int id : std::as_const(candidates))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Candidate with set full name:" << id << fullName;

        if      (parentId == -1)
        {
            return id;
        }
        else if (TagsCache::instance()->parentTag(id) == parentId)
        {
            return id;
        }
    }

    // Second attempt: Find by tag name.

    if (parentId == -1)
    {
        candidates = TagsCache::instance()->tagsForName(name);
    }
    else
    {
        tagId = TagsCache::instance()->tagForName(name, parentId);
        candidates.clear();

        if (tagId)
        {
            candidates << tagId;
        }
    }

    for (int id : std::as_const(candidates))
    {
        // Is this tag already a person tag?

        if      (FaceTags::isPerson(id))
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Found tag with name" << name << "is already a person." << id;

            return id;
        }
        else if (convert)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Converting tag with name" << name << "to a person." << id;
            makeFaceTag(id, fullName);

            return id;
        }
    }

    // Third: If desired, create a new tag.

    if (create)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Creating new tag for name" << name << "fullName" << fullName;

        if (parentId == -1)
        {
            parentId = FaceTags::personParentTag();
        }

        tagId = TagsCache::instance()->getOrCreateTag(tagPath(name, parentId));
        makeFaceTag(tagId, fullName);

        return tagId;
    }

    return 0;
}

// --- Public methods ---

QList<QString> FaceTags::allPersonNames()
{
    return TagsCache::instance()->tagNames(allPersonTags());
}

QList<QString> FaceTags::allPersonPaths()
{
    return TagsCache::instance()->tagPaths(allPersonTags());
}

int FaceTags::tagForPerson(const QString& name, int parentId, const QString& fullName)
{
    return FaceTagsHelper::tagForName(name, 0, parentId, fullName, false, false);
}

int FaceTags::getOrCreateTagForPerson(const QString& name, int parentId, const QString& fullName)
{
    return FaceTagsHelper::tagForName(name, 0, parentId, fullName, true, true);
}

void FaceTags::ensureIsPerson(int tagId, const QString& fullName)
{
    FaceTagsHelper::tagForName(QString(), tagId, 0, fullName, true, false);
}

bool FaceTags::isPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::person());
}

bool FaceTags::isTheUnknownPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson());
}

bool FaceTags::isTheUnconfirmedPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::unconfirmedPerson());
}

bool FaceTags::isTheIgnoredPerson(int tagId)
{
    return TagsCache::instance()->hasProperty(tagId, TagPropertyName::ignoredPerson());
}

QList<int> FaceTags::allPersonTags()
{
    return TagsCache::instance()->tagsWithProperty(TagPropertyName::person());
}

int FaceTags::scannedForFacesTagId()
{
    return TagsCache::instance()->getOrCreateInternalTag(InternalTagName::scannedForFaces()); // NOTE: no i18n here.
}

bool FaceTags::isSystemPersonTagId(int tagId)
{
    return (
            (tagId == ignoredPersonTagId())  ||
            (tagId == unknownPersonTagId())  ||
            (tagId == unconfirmedPersonTagId())
           );
}

QMultiMap<QString, QString> FaceTags::identityAttributes(int tagId)
{
    QMultiMap<QString, QString> attributes;
    QString uuid = TagsCache::instance()->propertyValue(tagId, TagPropertyName::faceEngineUuid());

    if (!uuid.isEmpty())
    {
        attributes.insert(QLatin1String("uuid"), uuid);
    }

    QString fullName = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());

    if (!fullName.isEmpty())
    {
        attributes.insert(QLatin1String("fullName"), fullName);
    }

    QString faceEngineName = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());
    QString tagName        = TagsCache::instance()->tagName(tagId);

    if (tagName != faceEngineName)
    {
        attributes.insert(QLatin1String("name"), faceEngineName);
        attributes.insert(QLatin1String("name"), tagName);
    }
    else
    {
        attributes.insert(QLatin1String("name"), tagName);
    }

    return attributes;
}

void FaceTags::applyTagIdentityMapping(int tagId, const QMultiMap<QString, QString>& attributes)
{
    TagProperties props(tagId);

    if (attributes.contains(QLatin1String("fullName")))
    {
        props.setProperty(TagPropertyName::person(), attributes.value(QLatin1String("fullName")));
    }

    // We do not change the digikam tag name at this point, but we have this extra tag property.

    if (attributes.contains(QLatin1String("name")))
    {
        props.setProperty(TagPropertyName::faceEngineName(), attributes.value(QLatin1String("name")));
    }

    props.setProperty(TagPropertyName::faceEngineUuid(), attributes.value(QLatin1String("uuid")));
}

int FaceTags::getOrCreateTagForIdentity(const QMultiMap<QString, QString>& attributes)
{
    // Attributes from FacesEngine's Identity object.
    // The text constants are defines in FacesEngine's API docs.

    if (attributes.isEmpty())
    {
        return FaceTags::unknownPersonTagId();
    }

    int tagId;

    // First, look for UUID.

    if (!attributes.value(QLatin1String("uuid")).isEmpty())
    {
        if (
            (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::faceEngineUuid(),
                                                              attributes.value(QLatin1String("uuid"))))
           )
        {
            return tagId;
        }
    }

    // Second, look for full name.

    if (!attributes.value(QLatin1String("fullName")).isEmpty())
    {
        if (
            (tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::person(),
                                                              attributes.value(QLatin1String("fullName"))))
           )
        {
            return tagId;
        }
    }

    // Third, look for either name or full name.
    // TODO: better support for "fullName".

    QString name = attributes.value(QLatin1String("name"));

    if (name.isEmpty())
    {
        name = attributes.value(QLatin1String("fullName"));
    }

    if (name.isEmpty())
    {
        return FaceTags::unknownPersonTagId();
    }

    if ((tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::faceEngineName(), name)))
    {
        return tagId;
    }

    if ((tagId = FaceTagsHelper::findFirstTagWithProperty(TagPropertyName::person(), name)))
    {
        return tagId;
    }

    // Identity is in FacesEngine's database, but not in ours, so create.

    tagId = FaceTagsHelper::tagForName(name, 0, -1, attributes.value(QLatin1String("fullName")), true, true);
    applyTagIdentityMapping(tagId, attributes);

    return tagId;
}

QString FaceTags::faceNameForTag(int tagId)
{
    if (!TagsCache::instance()->hasTag(tagId))
    {
        return QString();
    }

    QString id = TagsCache::instance()->propertyValue(tagId, TagPropertyName::person());

    if (id.isNull())
    {
        id = TagsCache::instance()->tagName(tagId);
    }

    return id;
}

int FaceTags::personParentTag()
{
    // Employ a heuristic.

    QList<int> personTags = allPersonTags();

    if (!personTags.isEmpty())
    {
        // We find the most toplevel parent tag of person tags.

        QMap<int, int> tiers;
        QMap<int, int>::const_iterator it;

        for (int tid : std::as_const(personTags))
        {
            QList<int> parents = TagsCache::instance()->parentTags(tid);

            if (!parents.isEmpty())
            {
                tiers.insert(parents.first(),
                             tiers.value(parents.first()) + 1);
            }
        }

        int count  = 0;
        int person = 0;

        for (it = tiers.constBegin() ; it != tiers.constEnd() ; ++it)
        {
            if (it.value() > count)
            {
                count  = it.value();
                person = it.key();
            }
        }

        if (person != 0)
        {
            return person;
        }
    }

    // Get or create default.

    QString i18nName = i18nc("People on your photos", "People");

    return TagsCache::instance()->getOrCreateTag(i18nName);
}

int FaceTags::unknownPersonTagId()
{
    QList<int> ids = TagsCache::instance()->tagsWithPropertyCached(TagPropertyName::unknownPerson());

    if (!ids.isEmpty())
    {
        return ids.first();
    }

    int unknownPersonTagId = TagsCache::instance()->getOrCreateTag(
                                        FaceTagsHelper::tagPath(
                                        i18nc("The list of detected faces from the collections but not recognized",
                                              "Unknown"),
                                        personParentTag()));

    TagProperties props(unknownPersonTagId);
    props.setProperty(TagPropertyName::person(),        QString()); // No name associated.
    props.setProperty(TagPropertyName::unknownPerson(), QString()); // Special property.

    return unknownPersonTagId;
}

int FaceTags::unconfirmedPersonTagId()
{
    QList<int> ids = TagsCache::instance()->tagsWithPropertyCached(TagPropertyName::unconfirmedPerson());

    if (!ids.isEmpty())
    {
        return ids.first();
    }

    int unknownPeopleTagId = TagsCache::instance()->getOrCreateTag(
                                        FaceTagsHelper::tagPath(
                                        i18nc("The list of recognized faces from the collections but not confirmed",
                                              "Unconfirmed"),
                                        personParentTag()));

    TagProperties props(unknownPeopleTagId);
    props.setProperty(TagPropertyName::person(),            QString()); // No name associated.
    props.setProperty(TagPropertyName::unconfirmedPerson(), QString()); // Special property.

    return unknownPeopleTagId;
}

int FaceTags::ignoredPersonTagId()
{
    QList<int> ids = TagsCache::instance()->tagsWithPropertyCached(TagPropertyName::ignoredPerson());

    if (!ids.isEmpty())
    {
        return ids.first();
    }

    int ignoredPersonTagId = TagsCache::instance()->getOrCreateTag(
                                        FaceTagsHelper::tagPath(
                                        i18nc("List of detected faces that need not be recognized",
                                              "Ignored"),
                                        personParentTag()));

    TagProperties props(ignoredPersonTagId);
    props.setProperty(TagPropertyName::person(),        QString());
    props.setProperty(TagPropertyName::ignoredPerson(), QString());

    return ignoredPersonTagId;
}

} // Namespace Digikam
