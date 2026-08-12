/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : Faces tags editor allowing easy manipulation of face tags
 *
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facetagseditor.h"

// Local includes

#include "coredbaccess.h"
#include "coredbconstants.h"
#include "coredbbackend.h"
#include "coredboperationgroup.h"
#include "iteminfo.h"
#include "itemtagpair.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"
#include "digikam_debug.h"

namespace Digikam
{

// --- Read from database ---

int FaceTagsEditor::faceCountForPersonInImage(qlonglong imageid, int tagId) const
{
    ItemTagPair pair(imageid, tagId);

    return pair.values(ImageTagPropertyName::tagRegion()).size();
}

QList<FaceTagsIface> FaceTagsEditor::databaseFaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::NormalFaces);
}

QList<FaceTagsIface> FaceTagsEditor::unconfirmedFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::UnconfirmedTypes);
}

QList<FaceTagsIface> FaceTagsEditor::unconfirmedNameFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::UnconfirmedName);
}

QList<FaceTagsIface> FaceTagsEditor::databaseFacesForTraining(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::FaceForTraining);
}

QList<FaceTagsIface> FaceTagsEditor::confirmedFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::ConfirmedName);
}

QList<FaceTagsIface> FaceTagsEditor::ignoredFaceTagsIfaces(qlonglong imageId) const
{
    return databaseFaces(imageId, FaceTagsIface::IgnoredName);
}

QList<FaceTagsIface> FaceTagsEditor::databaseFaces(qlonglong imageid, FaceTagsIface::TypeFlags flags) const
{
    QList<FaceTagsIface> faces;
    QStringList          attributes     = FaceTagsIface::attributesForFlags(flags);
    const auto           pairs          = faceItemTagPairs(imageid, flags);

    for (const ItemTagPair& pair : pairs)
    {
        for (const QString& attribute : std::as_const(attributes))
        {
            const auto regions = pair.values(attribute);

            for (const QString& regionString : regions)
            {
                TagRegion region(regionString);
/*
                qCDebug(DIGIKAM_DATABASE_LOG) << "rect found as "<< region << "for attribute"
                                              << attribute << "tag" << pair.tagId();
*/
                if (!region.isValid())
                {
                    continue;
                }

                // Load the rejected face tag list from the faceTagExtendedData Property.

                QList<int> rejectedFaceTagList = getRejectedFaceTagList(pair, region.toXml());

                // Add the face to the list.

                faces << FaceTagsIface(attribute, imageid, pair.tagId(), region, rejectedFaceTagList);
            }
        }
    }

    return faces;
}

QList<ItemTagPair> FaceTagsEditor::faceItemTagPairs(qlonglong imageid, FaceTagsIface::TypeFlags flags) const
{
    QList<ItemTagPair> pairs;
    QStringList        attributes = FaceTagsIface::attributesForFlags(flags);
    const auto images             = ItemTagPair::availablePairs(imageid);

    for (const ItemTagPair& pair : images)
    {
/*
        qCDebug(DIGIKAM_DATABASE_LOG) << pair.tagId() << pair.properties();
*/
        if (!FaceTags::isPerson(pair.tagId()))
        {
            continue;
        }

        // UnknownName and UnconfirmedName have the same attribute.

        if (!(flags & FaceTagsIface::UnknownName) && FaceTags::isTheUnknownPerson(pair.tagId()))
        {
            continue;
        }

        if (!pair.hasAnyProperty(attributes))
        {
            continue;
        }

        pairs << pair;
    }

    return pairs;
}

QList< QRect > FaceTagsEditor::getTagRects(qlonglong imageid) const
{
    QList<QRect>       rectList;
    QList<ItemTagPair> pairs = ItemTagPair::availablePairs(imageid);

    for (const ItemTagPair& pair : std::as_const(pairs))
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());

        for (const QString& region : std::as_const(regions))
        {
            QRect rect = TagRegion(region).toRect();

            if (rect.isValid())
            {
                rectList << rect;
            }
        }
    }

    return rectList;
}

int FaceTagsEditor::numberOfFaces(qlonglong imageid) const
{
    // Use case for this? Depending on a use case, we can think of an optimization.

    int                count = 0;
    QList<ItemTagPair> pairs = ItemTagPair::availablePairs(imageid);

    for (const ItemTagPair& pair : std::as_const(pairs))
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        count              += regions.size();
    }

    return count;
}

// --- Confirming and adding ---

FaceTagsIface FaceTagsEditor::unknownPersonEntry(qlonglong imageId, const TagRegion& region,
                                                 const QList<int>& rejectedFaceTagList)
{
    return unconfirmedEntry(imageId, -1, region, rejectedFaceTagList);
}

FaceTagsIface FaceTagsEditor::unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region,
                                               const QList<int>& rejectedFaceTagList)
{
    return FaceTagsIface(
                         FaceTagsIface::UnconfirmedName,
                         imageId,
                         (tagId == -1) ? FaceTags::unknownPersonTagId() : tagId,
                         region,
                         rejectedFaceTagList
                        );
}

FaceTagsIface FaceTagsEditor::confirmedEntry(const FaceTagsIface& face, int tagId, const TagRegion& confirmedRegion,
                                             const QList<int>& rejectedFaceTagList)
{
    return FaceTagsIface(
                         FaceTagsIface::ConfirmedName,
                         face.imageId(),
                         ((tagId == -1) ? face.tagId() : tagId),
                         (confirmedRegion.isValid() ? confirmedRegion : face.region()),
                         rejectedFaceTagList
                        );
}

FaceTagsIface FaceTagsEditor::addManually(const FaceTagsIface& face)
{
    ItemTagPair pair(face.imageId(), face.tagId());
    addFaceAndTag(pair, face, FaceTagsIface::attributesForFlags(face.type()), false);

    return face;
}

FaceTagsIface FaceTagsEditor::changeSuggestedName(const FaceTagsIface& previousEntry, int unconfirmedNameTagId)
{
    if (previousEntry.isConfirmedName())
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Refusing to reset a confirmed name to an unconfirmed name";

        return previousEntry;
    }

    FaceTagsIface newEntry = unconfirmedEntry(previousEntry.imageId(),
                                              unconfirmedNameTagId,
                                              previousEntry.region(),
                                              previousEntry.rejectedFaceTagList());

    if (newEntry == previousEntry)
    {
        return previousEntry;
    }

    removeFace(previousEntry);

    QStringList attributesList = FaceTagsIface::attributesForFlags(FaceTagsIface::UnconfirmedName);

    ItemTagPair pair(newEntry.imageId(), newEntry.tagId());

    // UnconfirmedName and UnknownName have the same attribute.

    addFaceAndTag(pair, newEntry, attributesList, false);

    // Add the image to the face to the unconfirmed tag, if it is not the unknown or unconfirmed tag.

    if (
        !FaceTags::isTheUnknownPerson(unconfirmedNameTagId) &&
        !FaceTags::isTheUnconfirmedPerson(unconfirmedNameTagId)
       )
    {
        ItemTagPair unconfirmedAssociation(newEntry.imageId(),
                                           FaceTags::unconfirmedPersonTagId());
        unconfirmedAssociation.addProperty(ImageTagPropertyName::autodetectedPerson(),
                                           newEntry.getAutodetectedPersonString());
    }

    return newEntry;
}

QMap<QString, QString> FaceTagsEditor::getSuggestedNames(qlonglong id) const
{
    QMap<QString, QString> suggestedNames;
    const auto pairs = ItemTagPair::availablePairs(id);

    for (const ItemTagPair& pair : pairs)
    {
        const auto regions = pair.values(ImageTagPropertyName::autodetectedPerson());

        for (const QString& regionString : regions)
        {
            /**
             * For Unconfirmed Results, the value is stored as a tuple of
             * (SuggestedId, Property, Region). Look at the digikam.db file
             * for more details.
             */

            QStringList valueList = regionString.split(QLatin1String(","));
            QString region(valueList.at(2));
            QString suggestedName = FaceTags::faceNameForTag(valueList.at(0).toInt());

            if (!TagRegion(region).isValid() || suggestedName.isEmpty())
            {
                continue;
            }

            suggestedNames.insert(region, suggestedName);
        }
    }

    return suggestedNames;
}

FaceTagsIface FaceTagsEditor::confirmName(const FaceTagsIface& face,
                                          int tagId,
                                          const TagRegion& confirmedRegion)
{
    FaceTagsIface newEntry = confirmedEntry(face, tagId, confirmedRegion, face.rejectedFaceTagList());

    if (
        FaceTags::isTheUnknownPerson(newEntry.tagId())     ||
        FaceTags::isTheUnconfirmedPerson(newEntry.tagId()) ||
        FaceTags::isTheIgnoredPerson(newEntry.tagId())
       )
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Refusing to confirm tag on face";

        return face;
    }

    ItemTagPair pair(newEntry.imageId(), newEntry.tagId());

    // Remove entry from autodetection.

    if (newEntry.tagId() == face.tagId())
    {
        removeFaceAndTag(pair, face, false);
    }
    else
    {
        ItemTagPair pairOldEntry(face.imageId(), face.tagId());
        removeFaceAndTag(pairOldEntry, face, true);
    }

    /**
     * Compare the original face tagId with the new one
     * if the face was suggested but a different tag was chosen
     * add the suggested face tag to the rejected list.
     */

    if (
        (face.tagId() != tagId) &&
        face.isUnconfirmedName()
       )
    {
        // Face was suggested but a different tag was chosen
        // so add the suggested face tag to the rejected list.

        newEntry.addRejectedFaceTag(face.tagId());
    }

    // Add new full entry.

    addFaceAndTag(pair, newEntry,
                  FaceTagsIface::attributesForFlags(FaceTagsIface::ConfirmedName | FaceTagsIface::FaceForTraining),
                  true);

    return newEntry;
}

FaceTagsIface FaceTagsEditor::add(qlonglong imageId, int tagId, const TagRegion& region, bool trainFace)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding face with rectangle  " << region.toRect () << " to database";

    FaceTagsIface newEntry(FaceTagsIface::ConfirmedName, imageId, tagId, region);
    add(newEntry, trainFace);

    return newEntry;
}

void FaceTagsEditor::add(const FaceTagsIface& face, bool trainFace)
{
    ItemTagPair pair(face.imageId(), face.tagId());
    FaceTagsIface::TypeFlags flags = FaceTagsIface::ConfirmedName;

    if (trainFace)
    {
        flags |= FaceTagsIface::FaceForTraining;
    }

    addFaceAndTag(pair, face, FaceTagsIface::attributesForFlags(flags), true);
}

void FaceTagsEditor::addFaceAndTag(ItemTagPair& pair,
                                   const FaceTagsIface& face,
                                   const QStringList& properties,
                                   bool addTag)
{
    FaceTags::ensureIsPerson(face.tagId());
    QString region = face.region().toXml();

    for (const QString& property : std::as_const(properties))
    {
        pair.addProperty(property, region);
    }

    addFaceTagExtendedDataProperty(pair, face);

    if (addTag)
    {
        addNormalTag(face.imageId(), face.tagId());
    }
}

// --- Removing faces ---

void FaceTagsEditor::removeAllFaces(qlonglong imageid)
{
    QList<int>  tagsToRemove;
    QStringList attributes = FaceTagsIface::attributesForFlags(FaceTagsIface::AllTypes);
    const auto pairs       = faceItemTagPairs(imageid, FaceTagsIface::AllTypes);

    // Remove the face tag extended data property because we are removing the face.

    attributes << ImageTagPropertyName::faceTagExtendedData();

    for (ItemTagPair pair : pairs)
    {
        for (const QString& attribute : std::as_const(attributes))
        {
            pair.removeProperties(attribute);
        }

        if (pair.isAssigned())
        {
            tagsToRemove << pair.tagId();
        }
    }

    removeNormalTags(imageid, tagsToRemove);
}

void FaceTagsEditor::removeFace(qlonglong imageid, const QRect& rect)
{
    QList<int>  tagsToRemove;
    QStringList attributes = FaceTagsIface::attributesForFlags(FaceTagsIface::AllTypes);
    const auto pairs       = faceItemTagPairs(imageid, FaceTagsIface::AllTypes);

    for (ItemTagPair pair : pairs)
    {
        removeFaceTagExtendedDataProperty(pair, TagRegion(rect).toXml());

        for (const QString& attribute : std::as_const(attributes))
        {
            const auto regions = pair.values(attribute);

            for (const QString& regionString : regions)
            {
                if (rect == TagRegion(regionString).toRect())
                {
                    pair.removeProperty(attribute, regionString);

                    if (pair.isAssigned())
                    {
                        tagsToRemove << pair.tagId();
                    }
                }
            }
        }
    }

    removeNormalTags(imageid, tagsToRemove);
}

void FaceTagsEditor::removeFace(const FaceTagsIface& face, bool touchTags)
{
    if (face.isNull())
    {
        return;
    }

    ItemTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, touchTags);
}

void FaceTagsEditor::removeFaces(const QList<FaceTagsIface>& faces)
{
    for (const FaceTagsIface& face : std::as_const(faces))
    {
        if (face.isNull())
        {
            continue;
        }

        ItemTagPair pair(face.imageId(), face.tagId());
        removeFaceAndTag(pair, face, true);
    }
}

void FaceTagsEditor::removeFaceAndTag(ItemTagPair& pair, const FaceTagsIface& face, bool touchTags)
{
    QString regionString = face.region().toXml();

    face.removeFaceTraining();

    pair.removeProperty(FaceTagsIface::attributeForType(face.type()), regionString);

    removeFaceTagExtendedDataProperty(pair, regionString);

    if (face.type() == FaceTagsIface::ConfirmedName)
    {
        pair.removeProperty(FaceTagsIface::attributeForType(FaceTagsIface::FaceForTraining), regionString);
    }

    // Remove the unconfirmed property for the image id and the unconfirmed tag
    // with the original tag id and the confirmed region.

    ItemTagPair unconfirmedAssociation(face.imageId(),
                                       FaceTags::unconfirmedPersonTagId());
    unconfirmedAssociation.removeProperty(ImageTagPropertyName::autodetectedPerson(),
                                          face.getAutodetectedPersonString());

    // Tag assigned and no other entry left?

    if      (
             touchTags            &&
             pair.isAssigned()    &&
             !pair.hasProperty(FaceTagsIface::attributeForType(FaceTagsIface::ConfirmedName))
            )
    {
        removeNormalTag(face.imageId(), pair.tagId());
    }
    else if (
             touchTags                                           &&
             (face.type() != FaceTagsIface::FaceForTraining)     &&
             pair.hasProperty(FaceTagsIface::attributeForType(FaceTagsIface::ConfirmedName))
            )
    {
        // The tag exists several times, we write it again
        // to trigger the writing of the metadata.

        addNormalTag(face.imageId(), pair.tagId());
    }
}

FaceTagsIface FaceTagsEditor::changeRegion(const FaceTagsIface& face, const TagRegion& newRegion)
{
    if (face.isNull() || (face.region() == newRegion))
    {
        return face;
    }

    ItemTagPair pair(face.imageId(), face.tagId());
    removeFaceAndTag(pair, face, false);

    FaceTagsIface newFace = face;
    newFace.setRegion(newRegion);
    addFaceAndTag(pair, newFace, FaceTagsIface::attributesForFlags(face.type()),
                  (face.type() == FaceTagsIface::ConfirmedName));

    if (face.type() == FaceTagsIface::UnconfirmedName)
    {
        ItemTagPair unconfirmedAssociation(newFace.imageId(),
                                           FaceTags::unconfirmedPersonTagId());
        unconfirmedAssociation.addProperty(ImageTagPropertyName::autodetectedPerson(),
                                           newFace.getAutodetectedPersonString());
    }

    return newFace;

    // TODO: the Training entry is cleared.
}

FaceTagsIface FaceTagsEditor::changeTag(const FaceTagsIface& face, int newTagId)
{
    if (face.isNull() || (face.tagId() == newTagId) || !FaceTags::isPerson(newTagId))
    {
        return face;
    }

    /**
     * Since a new Tag is going to be assigned to the Face,
     * it's important to remove the association between
     * the face and the old tagId.
     *
     * If the face is being ignored and it was an unconfirmed or
     * unknown face don't remove a possible tag. See bug 449142.
     */

    removeFace(face, (face.type() == FaceTagsIface::ConfirmedName));

    FaceTagsIface newFace = face;
    newFace.setTagId(newTagId);
    newFace.setType(FaceTagsIface::typeForId(newTagId));

    ItemTagPair newPair(newFace.imageId(), newFace.tagId());

    /**
     * We store metadata of FaceTags, if it's a confirmed person.
     */

    bool isConfirmed = (
                        !FaceTags::isTheIgnoredPerson(newTagId)  &&
                        !FaceTags::isTheUnknownPerson(newTagId)  &&
                        !FaceTags::isTheUnconfirmedPerson(newTagId)
                       );

    addFaceAndTag(newPair,
                  newFace,
                  FaceTagsIface::attributesForFlags(newFace.type()),
                  isConfirmed);

    return newFace;
}

FaceTagsIface FaceTagsEditor::rejectSuggestedTag(const FaceTagsIface& face)
{
    if (face.isNull() || !face.isUnconfirmedName())
    {
        // Process only unconfirmed faces.

        return face;
    }

    int rejectedTagId = face.tagId();

    /**
     * Since a new Tag is going to be assigned to the Face,
     * it's important to remove the association between
     * the face and the old tagId.
     *
     * If the face is being ignored and it was an unconfirmed or
     * unknown face don't remove a possible tag. See bug 449142.
     */

    removeFace(face, (face.type() == FaceTagsIface::ConfirmedName));

    FaceTagsIface newFace(face);
    newFace.setTagId(FaceTags::unknownPersonTagId());
    newFace.setType(FaceTagsIface::typeForId(FaceTags::unknownPersonTagId()));
    newFace.addRejectedFaceTag(rejectedTagId);

    ItemTagPair newPair(newFace.imageId(), newFace.tagId());

    addFaceAndTag(newPair,
                  newFace,
                  FaceTagsIface::attributesForFlags(newFace.type()),
                  false);

    return newFace;
}

bool FaceTagsEditor::rotateFaces(qlonglong imageId, const QSize& size,
                                 int oldOrientation, int newOrientation)
{
    bool hasConfirmed = false;

    // Get all faces from database and rotate them.

    QList<FaceTagsIface> facesList = databaseFaces(imageId);

    if (facesList.isEmpty())
    {
        return hasConfirmed;
    }

    QSize newSize = size;

    for (const FaceTagsIface& face : std::as_const(facesList))
    {
        hasConfirmed  |= face.isConfirmedName();
        QRect faceRect = face.region().toRect();

        TagRegion::reverseToOrientation(faceRect,
                                        oldOrientation, size);

        newSize = TagRegion::adjustToOrientation(faceRect,
                                                 newOrientation, size);

        changeRegion(face, TagRegion(faceRect));
    }

    return hasConfirmed;
}

// --- Editing normal tags ---

void FaceTagsEditor::addNormalTag(qlonglong imageId, int tagId)
{
    ItemInfo(imageId).setTag(tagId);
}

void FaceTagsEditor::removeNormalTag(qlonglong imageId, int tagId)
{
    ItemInfo(imageId).removeTag(tagId);
}

void FaceTagsEditor::removeNormalTags(qlonglong imageId, const QList<int>& tagIds)
{
    CoreDbOperationGroup group;
    group.setMaximumTime(200);
    ItemInfo info(imageId);

    for (int tagId : std::as_const(tagIds))
    {
        info.removeTag(tagId);
        group.allowLift();
    }
}

void FaceTagsEditor::removeFaceTagExtendedDataProperty(ItemTagPair& pair, const QString& regionString)
{
    if (pair.hasProperty(ImageTagPropertyName::faceTagExtendedData()))
    {
        QStringList values = pair.values(ImageTagPropertyName::faceTagExtendedData());

        for (const QString& value : std::as_const(values))
        {
            if (value.contains(regionString))
            {
                pair.removeProperty(ImageTagPropertyName::faceTagExtendedData(), value);
            }
        }
    }
}

void FaceTagsEditor::addFaceTagExtendedDataProperty(ItemTagPair& pair, const FaceTagsIface& face)
{
    if (!face.rejectedFaceTagList().isEmpty())
    {
        // Add the rejected list for the face.

        pair.addProperty(ImageTagPropertyName::faceTagExtendedData(), face.faceTagExtendedDataDBString());
    }
}

QList<int> FaceTagsEditor::getRejectedFaceTagList(const ItemTagPair& pair, const QString& regionString) const
{
    QList<int> rejectedFaceTagList;

    if (pair.hasProperty(ImageTagPropertyName::faceTagExtendedData()))
    {
        QStringList values = pair.values(ImageTagPropertyName::faceTagExtendedData());

        for (const QString& value : std::as_const(values))
        {
            if (value.contains(regionString))
            {
                QStringList list = value.split(FaceTagsIface::valueSeparator);

                if (list.size() > FaceTagsIface::ExtendedFaceData::RejectedFaceTagListPosition)
                {
                    // The first part is the region, the second part is the rejectedFaceTagList.

                    rejectedFaceTagList = FaceTagsIface::stringToRejectedFaceTagList(list.at(FaceTagsIface::ExtendedFaceData::RejectedFaceTagListPosition));
                }
                else
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "FaceTagsEditor::getRejectedFaceTagList: invalid rejectedFaceTagList"
                                                  << pair.value(ImageTagPropertyName::faceTagExtendedData());
                }
            }
        }
    }

    return rejectedFaceTagList;
}

void FaceTagsEditor::removeAllRejectedFaceTags()
{
    CoreDbOperationGroup group;
    group.setMaximumTime(200);

    QSqlQuery result = CoreDbAccess().backend()->execQuery(QLatin1String("SELECT imageid from ImageTagProperties where property='") +
                                                                         ImageTagPropertyName::faceTagExtendedData()                +
                                                                         QLatin1String("';"));
    while (result.next())
    {
        QList<FaceTagsIface> faces = databaseFaces(result.value(0).toLongLong());

        if (!faces.isEmpty())
        {
            for (const FaceTagsIface& face : std::as_const(faces))
            {
                if (face.isNull())
                {
                    continue;
                }

                ItemTagPair pair(face.imageId(), face.tagId());
                removeFaceTagExtendedDataProperty(pair, face.region().toXml());

                // TODO: Save this section for when we add more to faceTagExtendedData.
                // Add the extended data property again without the rejected face tags
                // if there is other data
/*
                FaceTagsIface newFace = face;
                newFace.clearRejectedFaceTagList();
                addFaceTagExtendedDataProperty(pair, newFace);
*/
            }
        }
    }
}

FaceTagsIface FaceTagsEditor::removeRejectedFaceTagList(const FaceTagsIface& face)
{
    if (face.isNull())
    {
        return face;
    }

    ItemTagPair pair(face.imageId(), face.tagId());
    removeFaceTagExtendedDataProperty(pair, face.region().toXml());

    FaceTagsIface newFace(face);
    newFace.clearRejectedFaceTagList();

    addFaceTagExtendedDataProperty(pair, newFace);

    return newFace;
}


} // Namespace Digikam
