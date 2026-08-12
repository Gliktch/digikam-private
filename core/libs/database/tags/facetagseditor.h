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

#pragma once

// Qt includes

#include <QFlags>
#include <QMap>
#include <QList>
#include <QRect>
#include <QString>
#include <QVariant>

// Local includes

#include "facetagsiface.h"
#include "digikam_export.h"
#include "facetags.h"

namespace Digikam
{

class ItemTagPair;
class ItemInfo;

class DIGIKAM_DATABASE_EXPORT FaceTagsEditor
{
public:

    FaceTagsEditor()          = default;
    virtual ~FaceTagsEditor() = default;

    // --- Read from database ----

    /**
     * Returns the number of faces present in an image.
     */
    int                  numberOfFaces(qlonglong imageid)                                       const;

    /**
     * Returns the number of faces a particular person has in the specified image.
     */
    int                  faceCountForPersonInImage(qlonglong imageid, int tagId)                const;

    /**
     * Reads the FaceTagsIfaces for the given image id from the database.
     */
    QList<FaceTagsIface> databaseFaces(qlonglong imageid)                                       const;

    /**
     * Returns list of Unconfirmed and Unknown faces in the Image.
     * If you want just Unconfirmed Faces, @see unconfirmedNameFaceTagsIfaces.
     */
    QList<FaceTagsIface> unconfirmedFaceTagsIfaces(qlonglong imageid)                           const;

    /**
     * Returns a list of UnconfirmedFaces in the Image.
     * Different from @see unconfirmedFaceTagsIfaces.
     */
    QList<FaceTagsIface> unconfirmedNameFaceTagsIfaces(qlonglong imageid)                       const;

    QList<FaceTagsIface> databaseFacesForTraining(qlonglong imageid)                            const;
    QList<FaceTagsIface> confirmedFaceTagsIfaces(qlonglong imageid)                             const;
    QList<FaceTagsIface> ignoredFaceTagsIfaces(qlonglong imageid)                               const;

    /**
     * Returns a list of all tag rectangles for the image. Unlike findAndTagFaces, this does not take a DImg,
     * because it returns only a QRect, not a Face, so no need of cropping a face rectangle.
     */
    QList<QRect>         getTagRects(qlonglong imageid)                                         const;

    // --- Add / Confirm ---

    /**
     * Adds a new entry to the database.
     * The convenience wrapper will return the newly created entry.
     * If trainFace is true, the face will also be listed in the db as needing training.
     * The tag of the face will, if necessary, be converted to a person tag.
     */
    void          add(const FaceTagsIface& face, bool trainFace = true);
    FaceTagsIface add(qlonglong imageid, int tagId, const TagRegion& region, bool trainFace = true);
    FaceTagsIface addManually(const FaceTagsIface& face);

    /**
     * Switches an unknownPersonEntry or unconfirmedEntry to an unconfirmedEntry (with a different suggested name).
     */
    FaceTagsIface changeSuggestedName(const FaceTagsIface& previousEntry, int unconfirmedNameTagId);

    /**
     * Returns a Map of Tag Regions (in XML format) to Suggested Name (from Face Recognition)
     * for the given image.
     * This function makes read operations to the database, and hence can be inefficient when
     * called repeatedly. A cached version is provided in ItemInfo, and should be preferred
     * for intensive operations such as sorting, categorizing etc.
     */
    QMap<QString, QString> getSuggestedNames(qlonglong id)                                      const;

    /**
     * Assign the name tag for given face entry.
     * Pass the tagId if it changed or was newly assigned (UnknownName).
     * Pass the new, corrected region if it changed.
     * If the default values are passed, tag id or region are taken from the given face.
     * The given face should be an unchanged entry read from the database.
     * The confirmed tag will, if necessary, be converted to a person tag.
     * Returns the newly inserted entry.
     */
    FaceTagsIface confirmName(const FaceTagsIface& face, int tagId = -1,
                              const TagRegion& confirmedRegion = TagRegion());

    /**
     * Returns the entry that would be added if the given face is confirmed.
     */
    static FaceTagsIface confirmedEntry(const FaceTagsIface& face, int tagId = -1,
                                        const TagRegion& confirmedRegion = TagRegion(),
                                        const QList<int>& rejectedFaceTagList = QList<int>());

    /**
     * Returns the entry that would be added if the given face is autodetected.
     * If tagId is -1, the unknown person will be taken.
     */
    static FaceTagsIface unconfirmedEntry(qlonglong imageId, int tagId, const TagRegion& region,
                                          const QList<int>& rejectedFaceTagList);
    static FaceTagsIface unknownPersonEntry(qlonglong imageId, const TagRegion& region,
                                            const QList<int>& rejectedFaceTagList);

    // --- Remove entries ---

    /**
     * Remove the given face.
     * If appropriate, the tag is also removed.
     */
    void                 removeFace(const FaceTagsIface& face, bool touchTags = true);
    void                 removeFaces(const QList<FaceTagsIface>& faces);

    /**
     * Unassigns all face tags from the image and sets it's scanned property to false.
     */
    void                 removeAllFaces(qlonglong imageid);

    /**
     * Remove a face or the face for a certain rect from an image.
     */
    void                 removeFace(qlonglong imageid, const QRect& rect);

    // --- Edit entry ---

    /**
     * Changes the region of the given entry. Returns the face with the new region set.
     */
    FaceTagsIface        changeRegion(const FaceTagsIface& face, const TagRegion& newRegion);

    /**
     * Changes the tag of the given entry. Returns the face with the new Tag.
     */
    FaceTagsIface        changeTag(const FaceTagsIface& face, int newTagId);

    /**
     * Changes the tag of the given entry to the unknown person tag and
     * adds the existing tag to the list of rejected face tags.
     */
    FaceTagsIface        rejectSuggestedTag(const FaceTagsIface& face);

    /**
     * Rotate face tags.
     */
    bool                 rotateFaces(qlonglong imageId, const QSize& size,
                                     int oldOrientation, int newOrientation);

    // --- Utilities ---

    QList<FaceTagsIface> databaseFaces(qlonglong imageId, FaceTagsIface::TypeFlags flags)       const;
    QList<ItemTagPair>   faceItemTagPairs(qlonglong imageid, FaceTagsIface::TypeFlags flags)    const;

    void                 removeAllRejectedFaceTags();
    FaceTagsIface        removeRejectedFaceTagList(const FaceTagsIface& face);

protected:

    void addFaceAndTag(ItemTagPair& pair, const FaceTagsIface& face,
                       const QStringList& properties, bool addTag);
    void removeFaceAndTag(ItemTagPair& pair, const FaceTagsIface& face, bool touchTags);


    void removeFaceTagExtendedDataProperty(ItemTagPair& pair, const QString& regionString);
    void addFaceTagExtendedDataProperty(ItemTagPair& pair, const FaceTagsIface& face);
    QList<int> getRejectedFaceTagList(const ItemTagPair& pair, const QString& regionString)     const;

    virtual void addNormalTag(qlonglong imageId, int tagId);
    virtual void removeNormalTag(qlonglong imageId, int tagId);
    virtual void removeNormalTags(qlonglong imageId, const QList<int>& tagIds);

private:

    Q_DISABLE_COPY(FaceTagsEditor)
};

} // namespace Digikam
