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

#include "facetagsiface.h"

// Qt includes

#include <QStringList>
#include <QCryptographicHash>
#include <QByteArray>

// Local includes

#include "digikam_debug.h"
#include "coredbconstants.h"
#include "tagscache.h"
#include "facetags.h"
#include "recognitiontrainingupdatequeue.h"

namespace Digikam
{

const QLatin1Char FaceTagsIface::listSeparator       = QLatin1Char(';');
const QLatin1Char FaceTagsIface::valueSeparator      = QLatin1Char(',');
const float FaceTagsIface::faceThumbnailResizeFactor = 0.25F;

FaceTagsIface::FaceTagsIface(const FaceTagsIface& other)
    : m_type   (other.type()),
      m_imageId(other.imageId()),
      m_tagId  (other.tagId()),
      m_region (other.region()),
      m_rejectedFaceTagList(other.m_rejectedFaceTagList)
{
}

FaceTagsIface::FaceTagsIface(Type type, qlonglong imageId, int tagId,
                             const TagRegion& region, const QList<int>& rejectedFaceTagList)
    : m_type   (type),
      m_imageId(imageId),
      m_tagId  (tagId),
      m_region (region),
      m_rejectedFaceTagList(rejectedFaceTagList)
{
}

FaceTagsIface::FaceTagsIface(const QString& attribute, qlonglong imageId,
                             int tagId, const TagRegion& region, const QList<int>& rejectedFaceTagList)
    : m_type   (typeForAttribute(attribute, tagId)),
      m_imageId(imageId),
      m_tagId  (tagId),
      m_region (region),
      m_rejectedFaceTagList(rejectedFaceTagList)
{
}

FaceTagsIface& FaceTagsIface::operator=(const FaceTagsIface& other)
{
    m_type                  = other.type();
    m_imageId               = other.imageId();
    m_tagId                 = other.tagId();
    m_region                = other.region();
    m_rejectedFaceTagList   = other.m_rejectedFaceTagList;

    return *this;
}

bool FaceTagsIface::isNull() const
{
    return (m_type == InvalidFace);
}

FaceTagsIface::Type FaceTagsIface::type() const
{
    return m_type;
}

qlonglong FaceTagsIface::imageId() const
{
    return m_imageId;
}

int FaceTagsIface::tagId() const
{
    return m_tagId;
}

TagRegion FaceTagsIface::region() const
{
    return m_region;
}

void FaceTagsIface::setType(Type type)
{
    m_type = type;
}

void FaceTagsIface::setTagId(int tagId)
{
    m_tagId = tagId;
}

void FaceTagsIface::setRegion(const TagRegion& region)
{
    m_region = region;
}

bool FaceTagsIface::operator==(const FaceTagsIface& other) const
{
    return (
            (m_tagId               == other.m_tagId)                &&
            (m_imageId             == other.m_imageId)              &&
            (m_type                == other.m_type)                 &&
            (m_region              == other.m_region)               &&
            (m_rejectedFaceTagList == other.m_rejectedFaceTagList)
           );
}

QStringList FaceTagsIface::attributesForFlags(TypeFlags flags)
{
    QStringList attributes;

    for (int i = FaceTagsIface::TypeFirst ; i <= FaceTagsIface::TypeLast ; i <<= 1)
    {
        if (flags & FaceTagsIface::Type(i))
        {
            QString attribute = attributeForType(FaceTagsIface::Type(i));

            if (!attributes.contains(attribute))
            {
                attributes << attribute;
            }
        }
    }

    return attributes;
}

QString FaceTagsIface::attributeForType(Type type)
{
    if (
        (type == FaceTagsIface::UnknownName) ||
        (type == FaceTagsIface::UnconfirmedName)
       )
    {
        return ImageTagPropertyName::autodetectedFace();
    }

    if (type == FaceTagsIface::ConfirmedName)
    {
        return ImageTagPropertyName::tagRegion();
    }

    if (type == FaceTagsIface::IgnoredName)
    {
        return ImageTagPropertyName::ignoredFace();
    }

    if (type == FaceTagsIface::FaceForTraining)
    {
        return ImageTagPropertyName::faceToTrain();
    }

    return QString();
}

FaceTagsIface::Type FaceTagsIface::typeForId(int tagId)
{
        if (!FaceTags::isPerson(tagId))
        {
            return InvalidFace;
        }

        if      (FaceTags::isTheUnknownPerson(tagId))
        {
            return UnknownName;
        }
        else if (FaceTags::isTheUnconfirmedPerson(tagId))
        {
            return UnconfirmedName;
        }
        else if (FaceTags::isTheIgnoredPerson(tagId))
        {
            return IgnoredName;
        }

        return ConfirmedName;
}

FaceTagsIface::Type FaceTagsIface::typeForAttribute(const QString& attribute, int tagId)
{
    if      (attribute == ImageTagPropertyName::autodetectedFace())
    {
        if (tagId && TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson()))
        {
            return FaceTagsIface::UnknownName;
        }
        else
        {
            return FaceTagsIface::UnconfirmedName;
        }
    }
    else if (attribute == ImageTagPropertyName::ignoredFace())
    {
        return FaceTagsIface::IgnoredName;
    }
    else if (attribute == ImageTagPropertyName::tagRegion())
    {
        return FaceTagsIface::ConfirmedName;
    }
    else if (attribute == ImageTagPropertyName::faceToTrain())
    {
        return FaceTagsIface::FaceForTraining;
    }

    return FaceTagsIface::InvalidFace;
}

FaceTagsIface FaceTagsIface::fromVariant(const QVariant& var)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (var.typeId() == qMetaTypeId<QList<QVariant> >())

#else

    if (var.type() == QVariant::List)

#endif

    {
        QList<QVariant> list(var.toList());

        if ((list.size() == 4) || (list.size() == 5))
        {
            return FaceTagsIface(
                                 (Type)list.at(0).toInt(),
                                 list.at(1).toLongLong(),
                                 list.at(2).toInt(),
                                 TagRegion::fromVariant(list.at(3)),
                                 QList<int>()
                                );
        }
    }

    return FaceTagsIface();
}

QVariant FaceTagsIface::toVariant() const
{
    // This is still not perfect, with QList<QVariant> being inefficient.
    // We must keep to native types, to make operator== work.

    QList<QVariant> list;
    list << (int)m_type;
    list << m_imageId;
    list << m_tagId;
    list << m_region.toVariant();

    return list;
}

FaceTagsIface FaceTagsIface::fromListing(qlonglong imageId, const QList<QVariant>& extraValues)
{
    if (extraValues.size() < 3)
    {
        return FaceTagsIface();
    }

    // See imagelister.cpp: value - property - tagId.

    int _tagId        = extraValues.at(2).toInt();
    QString attribute = extraValues.at(1).toString();
    QString value     = extraValues.at(0).toString();

/*
    qCDebug(DIGIKAM_DATABASE_LOG) << tagId << attribute << value;
*/
    return FaceTagsIface(
                         attribute,
                         imageId,
                         _tagId,
                         TagRegion(value),
                         QList<int>()
                        );
}

QString FaceTagsIface::getAutodetectedPersonString() const
{
    if (isUnconfirmedType())
    {
        return (
                QString::number(tagId())                 +
                QLatin1Char(',')                         +
                ImageTagPropertyName::autodetectedFace() +
                QLatin1Char(',')                         +
                (TagRegion(region().toRect())).toXml()
               );
    }
    else
    {
        return QString();
    }
}

QDebug operator<<(QDebug dbg, const FaceTagsIface& f)
{
    dbg.nospace() << "FaceTagsIface("       << f.type()
                  << ", image "             << f.imageId()
                  << ", tag "               << f.tagId()
                  << ", region"             << f.region()
                  << ", excluded tags"      << f.rejectedFaceTagList()
                  ;
    return dbg;
}

void FaceTagsIface::removeFaceTraining() const
{
    // We have to remove training for confirmed names only.

    if (ConfirmedName == m_type)
    {
        RecognitionTrainingUpdateQueue queue;

        queue.push(hash());
    }
}

/**
 * @note Feel free to optimize. QString::number is 3x slower.
 */
static inline QString fastNumberToString(qlonglong id)
{
    const int size   = sizeof(qlonglong) * 2;
    qlonglong number = id;
    char c[size + 1] = { 0 };
    c[size]          = '\0';

    for (int i = 0 ; i < size ; ++i)
    {
        c[i]     = 'a' + (number & 0xF);
        number >>= 4;
    }

    return QLatin1String(c);
}

const QString FaceTagsIface::hash() const
{
    // Create a unique hash consisting of the imageId, tagId and rect.

    QCryptographicHash hasher(QCryptographicHash::Sha1);

    hasher.addData(fastNumberToString(m_imageId).toLatin1());
    hasher.addData(fastNumberToString(m_tagId).toLatin1());
    hasher.addData(m_region.toXml().toLatin1());

    return QLatin1String(hasher.result().toHex());
}

bool FaceTagsIface::addRejectedFaceTag(int tagId)
{
    if (!m_rejectedFaceTagList.contains(tagId))
    {
        // check if the tagId is already in the list

        m_rejectedFaceTagList << tagId;
    }

    return true;
}

bool FaceTagsIface::setRejectedFaceTagList(const QList<int>& tagList)
{
    m_rejectedFaceTagList = tagList;

    return true;
}

void FaceTagsIface::clearRejectedFaceTagList()
{
    m_rejectedFaceTagList.clear();
}

const QList<int> FaceTagsIface::rejectedFaceTagList() const
{
    return m_rejectedFaceTagList;
}

const QString FaceTagsIface::rejectedFaceTagListToString() const
{
    QStringList strings;
    strings.reserve(m_rejectedFaceTagList.size());

    for (int i = 0 ; i < m_rejectedFaceTagList.size() ; ++i)
    {
        strings.append(QString::number(m_rejectedFaceTagList.at(i)));
    }

    return strings.join(listSeparator);
}

const QList<int> FaceTagsIface::stringToRejectedFaceTagList(const QString& str)
{
    QList<int> rejectedTags;
    QStringList strings = str.split(listSeparator);

    for (const QString& tagIdStr : std::as_const(strings))
    {
        bool ok;
        int _tagId = tagIdStr.toInt(&ok);

        if (ok)
        {
            rejectedTags << _tagId;
        }
    }

    return rejectedTags;
}

const QString FaceTagsIface::faceTagExtendedDataDBString() const
{
    return (
            m_region.toXml()              +
            FaceTagsIface::valueSeparator + 
            rejectedFaceTagListToString()
           );

           // other data can be added here later
}

} // namespace Digikam
