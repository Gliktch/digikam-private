/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - tags helpers.
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

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::getItemTagsPath(QStringList& tagsPath,
                                const DMetadataSettingsContainer& settings) const
{
    for (const NamespaceEntry& entry : EXIV2_AS_CONST(settings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER())))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        int index                                  = 0;
        QString currentNamespace                   = entry.namespaceName;
        NamespaceEntry::SpecialOptions currentOpts = entry.specialOpts;

        // Some namespaces have alternative paths, we must search them both

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                while (index < 2)
                {
                    const std::string myStr = currentNamespace.toStdString();
                    const char* nameSpace   = myStr.data();
                    QStringList xmpTagsPath;

                    switch (currentOpts)
                    {
                        case NamespaceEntry::TAG_XMPBAG:
                        {
                            xmpTagsPath = getXmpTagStringBag(nameSpace, false);
                            break;
                        }

                        case NamespaceEntry::TAG_XMPSEQ:
                        {
                            xmpTagsPath = getXmpTagStringSeq(nameSpace, false);
                            break;
                        }

                        case NamespaceEntry::TAG_ACDSEE:
                        {
                            getACDSeeTagsPath(xmpTagsPath);
                            break;
                        }

                        case NamespaceEntry::TAG_ALTLANG:
                        {
                            QString altlang = getXmpTagStringLangAlt(nameSpace, QString(), false);
                            xmpTagsPath.append(altlang.split(entry.separator));
                            break;
                        }

                        // not used here, to suppress warnings
                        case NamespaceEntry::COMMENT_XMP:
                        case NamespaceEntry::COMMENT_ALTLANG:
                        case NamespaceEntry::COMMENT_ATLLANGLIST:
                        case NamespaceEntry::NO_OPTS:
                        default:
                        {
                            break;
                        }
                    }

                    if      (!xmpTagsPath.isEmpty())
                    {
                        if (entry.separator != QLatin1String("/"))
                        {
                            xmpTagsPath.replaceInStrings(QLatin1String("/"), QLatin1String("\\"));
                            xmpTagsPath.replaceInStrings(entry.separator, QLatin1String("/"));
                        }

                        tagsPath.append(xmpTagsPath);

                        if (!settings.readingAllTags())
                        {
                            return true;
                        }
                    }

                    if (!entry.alternativeName.isEmpty())
                    {
                        currentNamespace = entry.alternativeName;
                        currentOpts      = entry.secondNameOpts;
                    }
                    else
                    {
                        break; // no alternative namespace, go to next one
                    }

                    index++;
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                // Try to get Tags Path list from IPTC keywords.
                // digiKam 0.9.x has used IPTC keywords to store Tags Path list.
                // This way is obsolete now since digiKam support XMP because IPTC
                // do not support UTF-8 and have strings size limitation. But we will
                // let the capability to import it for interworking issues.

                QStringList iptcTagsPath = getIptcKeywords();

                if (!iptcTagsPath.isEmpty())
                {
                    // Workaround to Imach tags path list hosted in IPTC with '.' as separator.
                    // Create a new entry with "." as a separator in the advanced metadata settings.

                    if (!entry.separator.isEmpty())
                    {
                        iptcTagsPath.replaceInStrings(entry.separator, QLatin1String("/"));
                    }

                    tagsPath.append(iptcTagsPath);

                    if (!settings.readingAllTags())
                    {
                        return true;
                    }
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                // Try to get Tags Path list from Exif Windows keywords.

                QString keyWords = getExifTagString("Exif.Image.XPKeywords", false);
                QStringList exifTagsPath;

                if (!keyWords.isEmpty())
                {
                    if (entry.tagPaths == NamespaceEntry::TAGPATH)
                    {
                        exifTagsPath = keyWords.split(QLatin1Char(';'));

                        if (entry.separator != QLatin1String("/"))
                        {
                            exifTagsPath.replaceInStrings(entry.separator, QLatin1String("/"));
                        }
                    }
                    else
                    {
                        exifTagsPath = keyWords.split(entry.separator);
                    }

                    if (!exifTagsPath.isEmpty())
                    {
                        tagsPath.append(exifTagsPath);

                        if (!settings.readingAllTags())
                        {
                            return true;
                        }
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    tagsPath.removeDuplicates();

    return (!tagsPath.isEmpty());
}

bool DMetadata::setItemTagsPath(const QStringList& tagsPath, const DMetadataSettingsContainer& settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Keywords for that.
    // Now this way is obsolete, and we use XMP instead.

    // Set the new Tags path list. This is set, not add-to like setXmpKeywords.
    // Unlike the other keyword fields, we do not need to merge existing entries.

    QList<NamespaceEntry> toWrite = settings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER());

    if (!settings.unifyReadWrite())
    {
        toWrite = settings.getWriteMapping(NamespaceEntry::DM_TAG_CONTAINER());
    }

    for (const NamespaceEntry& entry : EXIV2_AS_CONST(toWrite))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        QStringList newList;

        // Get keywords from tags path, for type tag

        for (const QString& tagPath : EXIV2_AS_CONST(tagsPath))
        {
            newList.append(tagPath.split(QLatin1Char('/')).last());
        }

        newList.removeDuplicates();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (!supportXmp())
                {
                    continue;
                }

                if (entry.tagPaths == NamespaceEntry::TAGPATH)
                {
                    newList = tagsPath;

                    if (entry.separator != QLatin1String("/"))
                    {
                        newList.replaceInStrings(QLatin1String("/"), entry.separator);
                    }
                }

                switch (entry.specialOpts)
                {
                    case NamespaceEntry::TAG_XMPSEQ:
                    {
                        if (!setXmpTagStringSeq(nameSpace, newList))
                        {
                            qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
                            return false;
                        }

                        break;
                    }

                    case NamespaceEntry::TAG_XMPBAG:
                    {
                        if (!setXmpTagStringBag(nameSpace, newList))
                        {
                            qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
                            return false;
                        }

                        break;
                    }

                    case NamespaceEntry::TAG_ACDSEE:
                    {
                        if (!setACDSeeTagsPath(newList))
                        {
                            qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
                            return false;
                        }

                        break;
                    }

                    case NamespaceEntry::TAG_ALTLANG:
                    {
                        if (!setXmpTagStringLangAlt(nameSpace, newList.join(entry.separator), QString()))
                        {
                            qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
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
                if (entry.namespaceName == QLatin1String("Iptc.Application2.Keywords"))
                {
                    if (!setIptcKeywords(getIptcKeywords(), newList))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
                        return false;
                    }
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (entry.namespaceName == QLatin1String("Exif.Image.XPKeywords"))
                {
                    if      (writeWithExifTool() && !newList.isEmpty())
                    {
                        QString xpKeywords;

                        if (entry.tagPaths == NamespaceEntry::TAGPATH)
                        {
                            newList = tagsPath;

                            if (entry.separator != QLatin1String("/"))
                            {
                                newList.replaceInStrings(QLatin1String("/"), entry.separator);
                            }

                            xpKeywords = newList.join(QLatin1Char(';'));
                        }
                        else
                        {
                            xpKeywords = newList.join(entry.separator);
                        }

                        QByteArray xpData  = QByteArray(reinterpret_cast<const char*>(xpKeywords.utf16()), xpKeywords.size() * 2);
                        xpData.append(2, '\0');

                        if (!setExifTagData(nameSpace, xpData))
                        {
                            qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image tags failed" << nameSpace;
                            return false;
                        }
                    }
                    else if (removeExifTag(nameSpace))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Remove image tags" << nameSpace;
                    }
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

bool DMetadata::getACDSeeTagsPath(QStringList& tagsPath) const
{
    // Try to get Tags Path list from ACDSee 8 Pro categories.

    QString xmlACDSee = getXmpTagString("Xmp.acdsee.categories", false);

    if (xmlACDSee.contains(QLatin1String("<Categories>")))
    {
        xmlACDSee.remove(QLatin1String("</Categories>"));
        xmlACDSee.remove(QLatin1String("<Categories>"));
        xmlACDSee.replace(QLatin1Char('/'), QLatin1Char('\\'));

        QStringList xmlTags = xmlACDSee.split(QLatin1String("<Category Assigned"));
        int category        = 0;

        for (const QString& tags : EXIV2_AS_CONST(xmlTags))
        {
            if (!tags.isEmpty())
            {
                int count  = tags.count(QLatin1String("<\\Category>"));
                int length = tags.length() - (11 * count) - 5;

                // cppcheck-suppress knownConditionTrueFalse
                if (category == 0)
                {
                    tagsPath << tags.mid(5, length);
                }
                else
                {
                    tagsPath.last().append(QLatin1Char('/') + tags.mid(5, length));
                }

                category = category - count + 1;

                if ((tags.left(5) == QLatin1String("=\"1\">")) && (category > 0))
                {
                    tagsPath << tagsPath.last().section(QLatin1Char('/'), 0, category - 1);
                }
            }
        }

        if (!tagsPath.isEmpty())
        {
/*
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from ACDSee: " << tagsPath;
*/
            return true;
        }
    }

    return false;
}

bool DMetadata::setACDSeeTagsPath(const QStringList& tagsPath) const
{
    // Converting Tags path list to ACDSee 8 Pro categories.

    const QString category(QLatin1String("<Category Assigned=\"%1\">"));
    QStringList splitTags;
    QStringList xmlTags;

    for (const QString& tags : EXIV2_AS_CONST(tagsPath))
    {
        splitTags   = tags.split(QLatin1Char('/'));
        int current = 0;

        for (int index = 0 ; index < splitTags.size() ; index++)
        {
            int tagIndex = xmlTags.indexOf(category.arg(0) + splitTags[index]);

            if (tagIndex == -1)
            {
                tagIndex = xmlTags.indexOf(category.arg(1) + splitTags[index]);
            }

            splitTags[index].insert(0, category.arg(index == splitTags.size() - 1 ? 1 : 0));

            if (tagIndex == -1)
            {
                if (index == 0)
                {
                    xmlTags << splitTags[index];
                    xmlTags << QLatin1String("</Category>");
                    current = xmlTags.size() - 1;
                }
                else
                {
                    xmlTags.insert(current, splitTags[index]);
                    xmlTags.insert(current + 1, QLatin1String("</Category>"));
                    current++;
                }
            }
            else
            {
                if (index == (splitTags.size() - 1))
                {
                    xmlTags[tagIndex] = splitTags[index];
                }

                current = tagIndex + 1;
            }
        }
    }

    QString xmlACDSee = QLatin1String("<Categories>") + xmlTags.join(QLatin1String("")) + QLatin1String("</Categories>");
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << "xmlACDSee" << xmlACDSee;
*/
    removeXmpTag("Xmp.acdsee.categories");

    if (!xmlTags.isEmpty())
    {
        if (!setXmpTagString("Xmp.acdsee.categories", xmlACDSee))
        {
            return false;
        }
    }

    return true;
}

} // namespace Digikam
