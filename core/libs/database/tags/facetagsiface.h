/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-27
 * Description : Interface for info stored about a face tag in the database
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>
#include <QVariant>
#include <QDebug>

// Local includes

#include "tagregion.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT FaceTagsIface
{
public:

    enum Type
    {
        InvalidFace      = 0,
        UnknownName      = 1 << 0,
        UnconfirmedName  = 1 << 1,
        IgnoredName      = 1 << 2,
        ConfirmedName    = 1 << 3,
        FaceForTraining  = 1 << 4,

        UnconfirmedTypes = UnknownName | UnconfirmedName,
        NormalFaces      = UnknownName | UnconfirmedName | IgnoredName | ConfirmedName,
        AllTypes         = UnknownName | UnconfirmedName | IgnoredName | ConfirmedName | FaceForTraining,
        TypeFirst        = UnknownName,
        TypeLast         = FaceForTraining
    };
    Q_DECLARE_FLAGS(TypeFlags, Type)

    enum ExtendedFaceData
    {
        RejectedFaceTagListPosition = 1
    };

public:

    FaceTagsIface() = default;
    FaceTagsIface(const FaceTagsIface& other);
    FaceTagsIface(Type type, qlonglong imageId, int tagId, const TagRegion& region,
                  const QList<int>& rejectedFaceTagList = QList<int>());
    FaceTagsIface(const QString& attribute, qlonglong imageId, int tagId,
                  const TagRegion& region, const QList<int>& rejectedFaceTagList = QList<int>());
    ~FaceTagsIface() = default;

    FaceTagsIface& operator=(const FaceTagsIface& other);

    bool        isNull()                          const;

    Type        type()                            const;
    qlonglong   imageId()                         const;
    int         tagId()                           const;
    TagRegion   region()                          const;

    /**
     * @return the list of tags excluded from face recognition
     * so "rejected" matches are not matched again
     */
    const QList<int>  rejectedFaceTagList()       const;

    bool      isInvalidFace()                     const
    {
        return (type() == InvalidFace);
    }

    bool      isUnknownName()                     const
    {
        return (type() == UnknownName);
    }

    bool      isUnconfirmedName()                 const
    {
        return (type() == UnconfirmedName);
    }

    bool      isUnconfirmedType()                 const
    {
        return (type() & UnconfirmedTypes);
    }

    bool      isIgnoredName()                     const
    {
        return (type() == IgnoredName);
    }

    bool      isConfirmedName()                   const
    {
        return (type() == ConfirmedName);
    }

    bool      isForTraining()                     const
    {
        return (type() == FaceForTraining);
    }

    void setType(Type type);
    void setTagId(int tagId);
    void setRegion(const TagRegion& region);

    bool operator==(const FaceTagsIface& other)   const;

    /**
     * @return a list of all image tag properties for which flags are set.
     */
    static QStringList attributesForFlags(TypeFlags flags);

    /**
     * @return the corresponding image tag property for the given type.
     */
    static QString attributeForType(Type type);

    /**
     * @return the Face Type corresponding to the given TagId.
     */
    static Type typeForId(int tagId);

    /**
     * @return the Type for the given attribute. To distinguish between UnknownName
     * and UnconfirmedName, the tagId must be given.
     */
    static Type typeForAttribute(const QString& attribute, int tagId = 0);

    /**
     * @return the string tagId + ',' + unconfirmedFace + ',' + regionXml.
     */
    QString getAutodetectedPersonString()       const;

    /**
     * @brief Writes the contents of this face - in a compact way - in the QVariant.
     * Only native QVariant types are used, that is, the QVariant will not have a custom type,
     * thus it can be compared by value by operator==.
     */
    static FaceTagsIface fromVariant(const QVariant& var);
    QVariant toVariant()                        const;

    /**
     * @brief Create a FaceTagsIface from the extraValues returned from ItemLister.
     */
    static FaceTagsIface fromListing(qlonglong imageid, const QList<QVariant>& values);

    /**
     * @brief Remove the face from face training based on the current imageId, tagId, and rect hash.
     */
    void removeFaceTraining()                   const;

    /**
     * @brief Generate a hash based on the imageId, tagId, and rect to uniquely identify this entry in the face training DB.
     */
    const QString hash()                        const;

    /**
     * @brief adds a tag to the list of tags excluded from face recognition
     * so "rejected" matches are not matched again
     */
    bool addRejectedFaceTag(int tagId);

    /**
     * @brief replaces the list of tags excluded from face recognition
     * Adds a tag to the list of tags excluded from face recognition
     * so "rejected" matches are not matched again.
     */
    bool setRejectedFaceTagList(const QList<int>& tagList);

    /**
     * @brief Clears the list of tags excluded from face recognition.
     */
    void clearRejectedFaceTagList();

    /**
     * @return a string representation of rejected face tag list
     * separated by list separator.
     */
    const QString rejectedFaceTagListToString() const;

    /**
     * @return a QList<int> from a string representation of m_rejectedFaceTagList
     * separated by listSeparator.
     */
    static const QList<int> stringToRejectedFaceTagList(const QString& str);

    /**
     * @return a string of the rect and the rejected face tag list to be saved in the DB.
     */
    const QString faceTagExtendedDataDBString() const;

public:

    static const QLatin1Char listSeparator;
    static const QLatin1Char valueSeparator;
    static const float       faceThumbnailResizeFactor;

protected:

    Type        m_type    = InvalidFace;
    qlonglong   m_imageId = 0;
    int         m_tagId   = 0;
    TagRegion   m_region;
    QList<int>  m_rejectedFaceTagList;
};

DIGIKAM_DATABASE_EXPORT QDebug operator<<(QDebug dbg, const FaceTagsIface& f);

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FaceTagsIface::TypeFlags)
