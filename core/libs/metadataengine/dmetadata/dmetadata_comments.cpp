/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - comments helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

CaptionsMap DMetadata::getItemComments(const DMetadataSettingsContainer& settings) const
{
    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap commentsMap;
    QString                commonAuthor;

    // In first try to get captions properties from digiKam XMP namespace

    if (supportXmp())
    {
        authorsMap = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames",    false);
        datesMap   = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", false);

        if (authorsMap.isEmpty())
        {
            QString xmpAuthors = getXmpTagString("Xmp.acdsee.author", false);

            if (!xmpAuthors.isEmpty())
            {
                authorsMap.insert(QLatin1String("x-default"), xmpAuthors);
            }
        }
    }

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.

    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
    {
        commonAuthor = descriptionWriter.toString();
    }

    // In first, we check XMP alternative language tags to create map of values.

    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (const NamespaceEntry& entry : EXIV2_AS_CONST(settings.getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER())))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        QString commentString;
        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                switch (entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                    {
                        if (xmpSupported)
                        {
                            commentString = getXmpTagStringLangAlt(nameSpace, QString(), false);
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                    {
                        if (xmpSupported)
                        {
                            commentsMap = getXmpTagStringListLangAlt(nameSpace, false);
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_XMP:
                    {
                        if (xmpSupported)
                        {
                            commentString = getXmpTagString(nameSpace, false);
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_JPEG:
                    {
                        // Now, we trying to get image comments, outside of XMP.
                        // For JPEG, string is extracted from JFIF Comments section.
                        // For PNG, string is extracted from iTXt chunk.

                        commentString = getCommentsDecoded();
                    }

                    default:
                    {
                        break;
                    }
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                if (iptcSupported)
                {
                    commentString = getIptcTagString(nameSpace, false);
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (exivSupported)
                {
                    commentString = getExifTagComment(nameSpace);

                    if (commentString.isEmpty() && !entry.alternativeName.isEmpty())
                    {
                        const std::string altStr   = entry.alternativeName.toStdString();
                        const char* alternateSpace = altStr.data();

                        commentString              = getExifTagComment(alternateSpace);
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if (!commentString.isEmpty() && !commentString.trimmed().isEmpty())
        {
            commentsMap.insert(QLatin1String("x-default"), commentString);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);

            return captionsMap;
        }

        if (!commentsMap.isEmpty())
        {
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);

            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setItemComments(const CaptionsMap& comments, const DMetadataSettingsContainer& settings) const
{
/*
    // See bug #139313: An empty string is also a valid value

    if (comments.isEmpty())
    {
          return false;
    }
*/

/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Comment: " << comments;
*/
    // In first, set captions properties to digiKam XMP namespace

    if (supportXmp())
    {
        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames", comments.authorsList()))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", comments.datesList()))
        {
            return false;
        }
    }

    QString defaultComment        = comments.value(QLatin1String("x-default")).caption;
    QList<NamespaceEntry> toWrite = settings.getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER());

    if (!settings.unifyReadWrite())
    {
        toWrite = settings.getWriteMapping(NamespaceEntry::DM_COMMENT_CONTAINER());
    }

    for (const NamespaceEntry& entry : EXIV2_AS_CONST(toWrite))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (!supportXmp())
                {
                    continue;
                }

                switch (entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                    {
                        removeXmpTag(nameSpace);

                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagStringLangAlt(nameSpace, defaultComment, QString()))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image comment failed" << nameSpace;
                                return false;
                            }
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                    {
                        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values

                        if (!setXmpTagStringListLangAlt(nameSpace, comments.toAltLangMap()))
                        {
                            return false;
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_XMP:
                    {
                        removeXmpTag(nameSpace);

                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagString(nameSpace, defaultComment))
                            {
                                return false;
                            }
                        }

                        if (entry.namespaceName == QLatin1String("Xmp.acdsee.notes"))
                        {
                            QString defaultAuthor = comments.value(QLatin1String("x-default")).author;
                            removeXmpTag("Xmp.acdsee.author");

                            if (!defaultAuthor.isNull())
                            {
                                if (!setXmpTagString("Xmp.acdsee.author", defaultAuthor))
                                {
                                    return false;
                                }
                            }
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_JPEG:
                    {
                        // In first we set image comments, outside of Exif, XMP, and IPTC.

                        if (!setComments(defaultComment.toUtf8()))
                        {
                            return false;
                        }

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                removeIptcTag(nameSpace);

                if (!defaultComment.isNull())
                {
                    defaultComment.truncate(2000);

                    if (!setIptcTagString(nameSpace, defaultComment))
                    {
                        return false;
                    }
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (entry.namespaceName == QLatin1String("Exif.Image.XPComment"))
                {
                    if      (writeWithExifTool() && !defaultComment.isEmpty())
                    {
                        QByteArray xpData  = QByteArray(reinterpret_cast<const char*>(defaultComment.utf16()), defaultComment.size() * 2);
                        xpData.append(2, '\0');

                        if (!setExifTagData(nameSpace, xpData))
                        {
                            return false;
                        }
                    }
                    else if (removeExifTag(nameSpace))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Remove image comment" << nameSpace;
                    }
                }
                else if (entry.namespaceName == QLatin1String("Exif.Photo.UserComment"))
                {
                    if (!setExifComment(defaultComment, false))
                    {
                        return false;
                    }
                }
                else if (!setExifTagString(nameSpace, defaultComment))
                {
                    return false;
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    return true;
}

CaptionsMap DMetadata::getItemTitles(const DMetadataSettingsContainer& settings) const
{
    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap titlesMap;
    QString                commonAuthor;

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.

    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
    {
        commonAuthor = descriptionWriter.toString();
    }

    // In first, we check XMP alternative language tags to create map of values.

    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (const NamespaceEntry& entry : EXIV2_AS_CONST(settings.getReadMapping(NamespaceEntry::DM_TITLE_CONTAINER())))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        QString titleString;
        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                switch (entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                    {
                        if (xmpSupported)
                        {
                            titlesMap = getXmpTagStringListLangAlt(nameSpace, false);
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_XMP:
                    {
                        if (xmpSupported)
                        {
                            titleString = getXmpTagString(nameSpace, false);
                        }

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                if (iptcSupported)
                {
                    titleString = getIptcTagString(nameSpace, false);
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (exivSupported)
                {
                    titleString = getExifTagString(nameSpace);
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if (!titleString.isEmpty() && !titleString.trimmed().isEmpty())
        {
            titlesMap.insert(QLatin1String("x-default"), titleString);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);

            return captionsMap;
        }

        if (!titlesMap.isEmpty())
        {
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);

            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setItemTitles(const CaptionsMap& titles, const DMetadataSettingsContainer& settings) const
{
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Title: " << titles;
*/
    QString defaultTitle          = titles[QLatin1String("x-default")].caption;
    QList<NamespaceEntry> toWrite = settings.getReadMapping(NamespaceEntry::DM_TITLE_CONTAINER());

    if (!settings.unifyReadWrite())
    {
        toWrite = settings.getWriteMapping(NamespaceEntry::DM_TITLE_CONTAINER());
    }

    for (const NamespaceEntry& entry : EXIV2_AS_CONST(toWrite))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (!supportXmp())
                {
                    continue;
                }

                switch (entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                    {
                        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values.

                        if (!setXmpTagStringListLangAlt(nameSpace, titles.toAltLangMap()))
                        {
                            return false;
                        }

                        break;
                    }

                    case NamespaceEntry::COMMENT_XMP:
                    {
                        removeXmpTag(nameSpace);

                        if (!defaultTitle.isNull())
                        {
                            if (!setXmpTagString(nameSpace, defaultTitle))
                            {
                                return false;
                            }
                        }

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                removeIptcTag(nameSpace);

                if (!defaultTitle.isNull())
                {
                    // Note that Caption IPTC tag is limited to 64 char and ASCII charset.

                    defaultTitle.truncate(64);

                    // See if we have any non printable chars in there. If so, skip IPTC
                    // to avoid confusing other apps and web services with invalid tags.

                    bool hasInvalidChar = false;

                    for (QString::const_iterator c = defaultTitle.constBegin() ; c != defaultTitle.constEnd() ; ++c)
                    {
                        if (!(*c).isPrint())
                        {
                            hasInvalidChar = true;
                            break;
                        }
                    }

                    if (!hasInvalidChar)
                    {
                        if (!setIptcTagString(nameSpace, defaultTitle))
                        {
                            return false;
                        }
                    }
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if      (entry.namespaceName == QLatin1String("Exif.Image.XPTitle"))
                {
                    if      (writeWithExifTool() && !defaultTitle.isEmpty())
                    {
                        QByteArray xpData  = QByteArray(reinterpret_cast<const char*>(defaultTitle.utf16()), defaultTitle.size() * 2);
                        xpData.append("\x00\x00");

                        if (!setExifTagData(nameSpace, xpData))
                        {
                            return false;
                        }
                    }
                    else if (removeExifTag(nameSpace))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Remove image title" << nameSpace;
                    }
                }
                else if (!setExifTagString(nameSpace, defaultTitle))
                {
                    return false;
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    return true;
}

MetaEngine::AltLangMap DMetadata::toAltLangMap(const QVariant& var)
{
    MetaEngine::AltLangMap map;

    if (var.isNull())
    {
        return map;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    switch (var.typeId())

#else

    switch (var.type())

#endif

    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QString:

#else

        case QVariant::String:

#endif

        {
            map.insert(QLatin1String("x-default"), var.toString());
            break;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case  qMetaTypeId<QMap<QString, QVariant> >():

#else

        case QVariant::Map:

#endif

        {
            QMap<QString, QVariant> varMap = var.toMap();

            for (QMap<QString, QVariant>::const_iterator it = varMap.constBegin() ; it != varMap.constEnd() ; ++it)
            {
                map.insert(it.key(), it.value().toString());
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return map;
}

} // namespace Digikam
