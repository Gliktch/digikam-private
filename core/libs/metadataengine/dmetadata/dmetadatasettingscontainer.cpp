/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : metadata Settings Container.
 *
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadatasettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dmetadatasettings.h"
#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

static const struct NameSpaceDefinition
{
    NamespaceEntry::NamespaceType type    = NamespaceEntry::TAGS;

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    const KLazyLocalizedString    title;

#else

    const char*                   context = nullptr;
    const char*                   title   = nullptr;

#endif

}
namespaceTitleDefinitions[] =
{
    {
        NamespaceEntry::TAGS,
        I18NC_NOOP("@title: xmp namespace for tags container",        "Tags")
    },
    {
        NamespaceEntry::TITLE,
        I18NC_NOOP("@title: xmp namespace for title container",       "Title")
    },
    {
        NamespaceEntry::RATING,
        I18NC_NOOP("@title: xmp namespace for rating container",      "Rating")
    },
    {
        NamespaceEntry::COMMENT,
        I18NC_NOOP("@title: xmp namespace for comment container",     "Caption")
    },
    {
        NamespaceEntry::PICKLABEL,
        I18NC_NOOP("@title: xmp namespace for pick-label container",  "Pick label")
    },
    {
        NamespaceEntry::COLORLABEL,
        I18NC_NOOP("@title: xmp namespace for color-label container", "Color label")
    },
};

QString NamespaceEntry::DM_TAG_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::TAGS].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::TAGS].title);

#endif

}

QString NamespaceEntry::DM_TITLE_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::TITLE].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::TITLE].title);

#endif

}

QString NamespaceEntry::DM_RATING_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::RATING].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::RATING].title);

#endif

}

QString NamespaceEntry::DM_COMMENT_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::COMMENT].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::COMMENT].title);

#endif

}

QString NamespaceEntry::DM_PICKLABEL_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::PICKLABEL].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::PICKLABEL].title);

#endif

}

QString NamespaceEntry::DM_COLORLABEL_CONTAINER()
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::COLORLABEL].title.untranslatedText());

#else

    return QString::fromUtf8(namespaceTitleDefinitions[NamespaceEntry::COLORLABEL].title);

#endif

}

// ------------------------------------------------------------

bool s_dmcompare(const NamespaceEntry& e1, const NamespaceEntry& e2)
{
    return (e1.index < e2.index);
}

QDebug operator<<(QDebug dbg, const NamespaceEntry& inf)
{
    dbg.nospace() << "[NamespaceEntry] nsType("
                  << inf.nsType << "), ";
    dbg.nospace() << "subspace("
                  << inf.subspace << "), ";
    dbg.nospace() << "isDefault("
                  << inf.isDefault << "), ";
    dbg.nospace() << "isDisabled("
                  << inf.isDisabled << "), ";
    dbg.nospace() << "index("
                  << inf.index << "), ";
    dbg.nospace() << "namespaceName("
                  << inf.namespaceName << "), ";
    dbg.nospace() << "alternativeName("
                  << inf.alternativeName << "), ";
    dbg.nospace() << "tagPaths("
                  << inf.tagPaths << "), ";
    dbg.nospace() << "separator("
                  << inf.separator << "), ";
    dbg.nospace() << "convertRatio("
                  << inf.convertRatio << "), ";
    dbg.nospace() << "specialOpts("
                  << inf.specialOpts << "), ";
    dbg.nospace() << "secondNameOpts("
                  << inf.secondNameOpts << ")";

    return dbg.space();
}

// -------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DMetadataSettingsContainer::Private
{
public:

    Private() = default;

public:

    QMap<QString, QList<NamespaceEntry> > readMappings;
    QMap<QString, QList<NamespaceEntry> > writeMappings;
    bool                                  unifyReadWrite = false;
    bool                                  readingAllTags = false;
};

// -------------------------------------------------------------------------------------------------

DMetadataSettingsContainer::DMetadataSettingsContainer()
    : d(new Private)
{
    addMapping(NamespaceEntry::DM_TAG_CONTAINER());
    addMapping(NamespaceEntry::DM_TITLE_CONTAINER());
    addMapping(NamespaceEntry::DM_RATING_CONTAINER());
    addMapping(NamespaceEntry::DM_COMMENT_CONTAINER());
    addMapping(NamespaceEntry::DM_PICKLABEL_CONTAINER());
    addMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER());
}

DMetadataSettingsContainer::DMetadataSettingsContainer(const DMetadataSettingsContainer& other)
    : d(new Private)
{
    *d = *other.d;
}

DMetadataSettingsContainer& DMetadataSettingsContainer::operator=(const DMetadataSettingsContainer& other)
{
    *d = *other.d;

    return *this;
}

DMetadataSettingsContainer::~DMetadataSettingsContainer()
{
    delete d;
}

bool DMetadataSettingsContainer::unifyReadWrite() const
{
    return d->unifyReadWrite;
}

void DMetadataSettingsContainer::setUnifyReadWrite(bool b)
{
    d->unifyReadWrite = b;
}

bool DMetadataSettingsContainer::readingAllTags() const
{
    return d->readingAllTags;
}

void DMetadataSettingsContainer::setReadingAllTags(bool b)
{
    d->readingAllTags = b;
}

void DMetadataSettingsContainer::readFromConfig(KConfigGroup& group)
{
    bool valid                   = true;
    const QString readNameSpace  = QLatin1String("read%1Namespaces");
    const QString writeNameSpace = QLatin1String("write%1Namespaces");
    const auto keys              = mappingKeys();

    for (const QString& str : EXIV2_AS_CONST(keys))
    {
        if (!group.hasGroup(readNameSpace.arg(str)))
        {
            valid = false;
            qCDebug(DIGIKAM_GENERAL_LOG) << "Does not contain " << str << " Namespace";
            break;
        }

        if (!group.hasGroup(writeNameSpace.arg(str)))
        {
            valid = false;
            qCDebug(DIGIKAM_GENERAL_LOG) << "Does not contain " << str << " Namespace";
            break;
        }
    }

    if (valid)
    {
        const auto keys2 = mappingKeys();

        for (const QString& str : EXIV2_AS_CONST(keys2))
        {
            readOneGroup(group, readNameSpace.arg(str),  getReadMapping(str));
            readOneGroup(group, writeNameSpace.arg(str), getWriteMapping(str));
        }
    }
    else
    {
        defaultValues();
    }

    d->unifyReadWrite = group.readEntry(QLatin1String("unifyReadWrite"), true);
    d->readingAllTags = group.readEntry(QLatin1String("readingAllTags"), false);
}

void DMetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    const QString readNameSpace  = QLatin1String("read%1Namespaces");
    const QString writeNameSpace = QLatin1String("write%1Namespaces");

    // Remove all old group elements.

    const auto list = group.groupList();

    for (const QString& groupKey : EXIV2_AS_CONST(list))
    {
        group.deleteGroup(groupKey);
    }

    const auto keys = mappingKeys();

    for (const QString& str : EXIV2_AS_CONST(keys))
    {
        writeOneGroup(group, readNameSpace.arg(str),  getReadMapping(str));
        writeOneGroup(group, writeNameSpace.arg(str), getWriteMapping(str));
    }

    group.writeEntry(QLatin1String("unifyReadWrite"), d->unifyReadWrite);
    group.writeEntry(QLatin1String("readingAllTags"), d->readingAllTags);
    group.sync();
}

void DMetadataSettingsContainer::defaultValues()
{
    d->unifyReadWrite = true;
    d->readingAllTags = false;
    d->writeMappings.clear();
    d->readMappings.clear();

    defaultTagValues();
    defaultTitleValues();
    defaultRatingValues();
    defaultCommentValues();
    defaultPickLabelValues();
    defaultColorLabelValues();
}

void DMetadataSettingsContainer::addMapping(const QString& key)
{
    d->readMappings[key]  = QList<NamespaceEntry>();
    d->writeMappings[key] = QList<NamespaceEntry>();
}

QList<NamespaceEntry>& DMetadataSettingsContainer::getReadMapping(const QString& key) const
{
    return d->readMappings[key];
}

QList<NamespaceEntry>& DMetadataSettingsContainer::getWriteMapping(const QString& key) const
{
    return d->writeMappings[key];
}

QList<QString> DMetadataSettingsContainer::mappingKeys() const
{
    return d->readMappings.keys();
}

void DMetadataSettingsContainer::defaultTagValues()
{
    // Default tag namespaces

    NamespaceEntry tagNs1;
    tagNs1.namespaceName    = QLatin1String("Xmp.digiKam.TagsList");
    tagNs1.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs1.separator        = QLatin1Char('/');
    tagNs1.nsType           = NamespaceEntry::TAGS;
    tagNs1.index            = 0;
    tagNs1.specialOpts      = NamespaceEntry::TAG_XMPSEQ;
    tagNs1.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName    = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs2.separator        = QLatin1Char('/');
    tagNs2.nsType           = NamespaceEntry::TAGS;
    tagNs2.index            = 1;
    tagNs2.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName    = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs3.separator        = QLatin1Char('|');
    tagNs3.nsType           = NamespaceEntry::TAGS;
    tagNs3.index            = 2;
    tagNs3.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace         = NamespaceEntry::XMP;
    tagNs3.alternativeName  = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts   = NamespaceEntry::TAG_XMPSEQ;

    NamespaceEntry tagNs4;
    tagNs4.namespaceName    = QLatin1String("Xmp.mediapro.CatalogSets");
    tagNs4.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs4.separator        = QLatin1Char('|');
    tagNs4.nsType           = NamespaceEntry::TAGS;
    tagNs4.index            = 3;
    tagNs4.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs4.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs5;
    tagNs5.namespaceName    = QLatin1String("Xmp.acdsee.categories");
    tagNs5.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs5.separator        = QLatin1Char('/');
    tagNs5.nsType           = NamespaceEntry::TAGS;
    tagNs5.index            = 4;
    tagNs5.specialOpts      = NamespaceEntry::TAG_ACDSEE;
    tagNs5.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs6;
    tagNs6.namespaceName    = QLatin1String("Xmp.dc.subject");
    tagNs6.tagPaths         = NamespaceEntry::TAG;
    tagNs6.separator        = QLatin1Char('/');
    tagNs6.nsType           = NamespaceEntry::TAGS;
    tagNs6.index            = 5;
    tagNs6.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs6.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs7;
    tagNs7.namespaceName    = QLatin1String("Iptc.Application2.Keywords");
    tagNs7.tagPaths         = NamespaceEntry::TAG;
    tagNs7.nsType           = NamespaceEntry::TAGS;
    tagNs7.index            = 6;
    tagNs7.subspace         = NamespaceEntry::IPTC;

    NamespaceEntry tagNs8;
    tagNs8.namespaceName    = QLatin1String("Exif.Image.XPKeywords");
    tagNs8.tagPaths         = NamespaceEntry::TAG;
    tagNs8.separator        = QLatin1Char(';');
    tagNs8.nsType           = NamespaceEntry::TAGS;
    tagNs8.index            = 7;
    tagNs8.subspace         = NamespaceEntry::EXIF;

    getReadMapping(NamespaceEntry::DM_TAG_CONTAINER()) << tagNs1
                                                       << tagNs2
                                                       << tagNs3
                                                       << tagNs4
                                                       << tagNs5
                                                       << tagNs6
                                                       << tagNs7
                                                       << tagNs8;

    d->writeMappings[NamespaceEntry::DM_TAG_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_TAG_CONTAINER()));
}

void DMetadataSettingsContainer::defaultTitleValues()
{
    NamespaceEntry titleNs1;
    titleNs1.namespaceName  = QLatin1String("Xmp.dc.title");
    titleNs1.nsType         = NamespaceEntry::TITLE;
    titleNs1.specialOpts    = NamespaceEntry::COMMENT_ATLLANGLIST;
    titleNs1.index          = 0;
    titleNs1.subspace       = NamespaceEntry::XMP;

    NamespaceEntry titleNs2;
    titleNs2.namespaceName  = QLatin1String("Xmp.acdsee.caption");
    titleNs2.nsType         = NamespaceEntry::TITLE;
    titleNs2.specialOpts    = NamespaceEntry::COMMENT_XMP;
    titleNs2.index          = 1;
    titleNs2.subspace       = NamespaceEntry::XMP;

    NamespaceEntry titleNs3;
    titleNs3.namespaceName  = QLatin1String("Iptc.Application2.ObjectName");
    titleNs3.nsType         = NamespaceEntry::TITLE;
    titleNs3.specialOpts    = NamespaceEntry::NO_OPTS;
    titleNs3.index          = 2;
    titleNs3.subspace       = NamespaceEntry::IPTC;

    NamespaceEntry titleNs4;
    titleNs4.namespaceName  = QLatin1String("Exif.Image.XPTitle");
    titleNs4.nsType         = NamespaceEntry::TITLE;
    titleNs4.specialOpts    = NamespaceEntry::NO_OPTS;
    titleNs4.index          = 3;
    titleNs4.subspace       = NamespaceEntry::EXIF;

    getReadMapping(NamespaceEntry::DM_TITLE_CONTAINER()) << titleNs1
                                                         << titleNs2
                                                         << titleNs3
                                                         << titleNs4;

    d->writeMappings[NamespaceEntry::DM_TITLE_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_TITLE_CONTAINER()));
}

void DMetadataSettingsContainer::defaultRatingValues()
{
    QList<int> defaultVal;
    QList<int> microsoftMappings;

    defaultVal        << 0 << 1 << 2  << 3  << 4  << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;

    NamespaceEntry ratingNs1;
    ratingNs1.namespaceName = QLatin1String("Xmp.xmp.Rating");
    ratingNs1.convertRatio  = defaultVal;
    ratingNs1.nsType        = NamespaceEntry::RATING;
    ratingNs1.index         = 0;
    ratingNs1.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs4;
    ratingNs4.namespaceName = QLatin1String("Exif.Image.Rating");
    ratingNs4.convertRatio  = defaultVal;
    ratingNs4.nsType        = NamespaceEntry::RATING;
    ratingNs4.index         = 3;
    ratingNs4.subspace      = NamespaceEntry::EXIF;

    NamespaceEntry ratingNs5;
    ratingNs5.namespaceName = QLatin1String("Exif.Image.RatingPercent");
    ratingNs5.convertRatio  = microsoftMappings;
    ratingNs5.nsType        = NamespaceEntry::RATING;
    ratingNs5.index         = 4;
    ratingNs5.subspace      = NamespaceEntry::EXIF;

    getReadMapping(NamespaceEntry::DM_RATING_CONTAINER()) << ratingNs1
                                                          << ratingNs2
                                                          << ratingNs3
                                                          << ratingNs4
                                                          << ratingNs5;

    d->writeMappings[NamespaceEntry::DM_RATING_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_RATING_CONTAINER()));
}

void DMetadataSettingsContainer::defaultCommentValues()
{
    NamespaceEntry commNs1;
    commNs1.namespaceName   = QLatin1String("Xmp.dc.description");
    commNs1.nsType          = NamespaceEntry::COMMENT;
    commNs1.specialOpts     = NamespaceEntry::COMMENT_ATLLANGLIST;
    commNs1.index           = 0;
    commNs1.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs2;
    commNs2.namespaceName   = QLatin1String("Xmp.exif.UserComment");
    commNs2.nsType          = NamespaceEntry::COMMENT;
    commNs2.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs2.index           = 1;
    commNs2.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs3;
    commNs3.namespaceName   = QLatin1String("Xmp.tiff.ImageDescription");
    commNs3.nsType          = NamespaceEntry::COMMENT;
    commNs3.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs3.index           = 2;
    commNs3.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs4;
    commNs4.namespaceName   = QLatin1String("Xmp.acdsee.notes");
    commNs4.nsType          = NamespaceEntry::COMMENT;
    commNs4.specialOpts     = NamespaceEntry::COMMENT_XMP;
    commNs4.index           = 3;
    commNs4.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs5;
    commNs5.namespaceName   = QLatin1String("JPEG/TIFF Comments");
    commNs5.nsType          = NamespaceEntry::COMMENT;
    commNs5.specialOpts     = NamespaceEntry::COMMENT_JPEG;
    commNs5.index           = 4;
    commNs5.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs6;
    commNs6.namespaceName   = QLatin1String("Exif.Image.ImageDescription");
    commNs6.nsType          = NamespaceEntry::COMMENT;
    commNs6.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs6.index           = 5;
    commNs6.subspace        = NamespaceEntry::EXIF;

    NamespaceEntry commNs7;
    commNs7.namespaceName   = QLatin1String("Exif.Photo.UserComment");
    commNs7.nsType          = NamespaceEntry::COMMENT;
    commNs7.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs7.index           = 6;
    commNs7.subspace        = NamespaceEntry::EXIF;

    NamespaceEntry commNs8;
    commNs8.namespaceName   = QLatin1String("Exif.Image.XPComment");
    commNs8.nsType          = NamespaceEntry::COMMENT;
    commNs8.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs8.index           = 7;
    commNs8.subspace        = NamespaceEntry::EXIF;

    NamespaceEntry commNs9;
    commNs9.namespaceName   = QLatin1String("Iptc.Application2.Caption");
    commNs9.nsType          = NamespaceEntry::COMMENT;
    commNs9.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs9.index           = 8;
    commNs9.subspace        = NamespaceEntry::IPTC;

    getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER()) << commNs1
                                                           << commNs2
                                                           << commNs3
                                                           << commNs4
                                                           << commNs5
                                                           << commNs6
                                                           << commNs7
                                                           << commNs8
                                                           << commNs9;

    d->writeMappings[NamespaceEntry::DM_COMMENT_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER()));
}

void DMetadataSettingsContainer::defaultPickLabelValues()
{
    NamespaceEntry pickNs1;
    pickNs1.namespaceName   = QLatin1String("Xmp.digiKam.PickLabel");
    pickNs1.nsType          = NamespaceEntry::PICKLABEL;
    pickNs1.specialOpts     = NamespaceEntry::NO_OPTS;
    pickNs1.index           = 0;
    pickNs1.subspace        = NamespaceEntry::XMP;

    NamespaceEntry pickNs2;
    pickNs2.namespaceName   = QLatin1String("Xmp.xmpDM.pick");
    pickNs2.nsType          = NamespaceEntry::PICKLABEL;
    pickNs2.specialOpts     = NamespaceEntry::NO_OPTS;
    pickNs2.index           = 1;
    pickNs2.subspace        = NamespaceEntry::XMP;

    NamespaceEntry pickNs3;
    pickNs3.namespaceName   = QLatin1String("Xmp.xmpDM.good");
    pickNs3.nsType          = NamespaceEntry::PICKLABEL;
    pickNs3.specialOpts     = NamespaceEntry::NO_OPTS;
    pickNs3.index           = 2;
    pickNs3.subspace        = NamespaceEntry::XMP;

    getReadMapping(NamespaceEntry::DM_PICKLABEL_CONTAINER()) << pickNs1
                                                             << pickNs2
                                                             << pickNs3;

    d->writeMappings[NamespaceEntry::DM_PICKLABEL_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_PICKLABEL_CONTAINER()));
}

void DMetadataSettingsContainer::defaultColorLabelValues()
{
    NamespaceEntry colorNs1;
    colorNs1.namespaceName   = QLatin1String("Xmp.digiKam.ColorLabel");
    colorNs1.nsType          = NamespaceEntry::COLORLABEL;
    colorNs1.specialOpts     = NamespaceEntry::NO_OPTS;
    colorNs1.index           = 0;
    colorNs1.subspace        = NamespaceEntry::XMP;

    NamespaceEntry colorNs2;
    colorNs2.namespaceName   = QLatin1String("Xmp.xmp.Label");
    colorNs2.nsType          = NamespaceEntry::COLORLABEL;
    colorNs2.specialOpts     = NamespaceEntry::NO_OPTS;
    colorNs2.index           = 1;
    colorNs2.subspace        = NamespaceEntry::XMP;

    NamespaceEntry colorNs3;
    colorNs3.namespaceName   = QLatin1String("Xmp.photoshop.Urgency");
    colorNs3.nsType          = NamespaceEntry::COLORLABEL;
    colorNs3.specialOpts     = NamespaceEntry::NO_OPTS;
    colorNs3.index           = 2;
    colorNs3.subspace        = NamespaceEntry::XMP;

    getReadMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER()) << colorNs1
                                                              << colorNs2
                                                              << colorNs3;

    d->writeMappings[NamespaceEntry::DM_COLORLABEL_CONTAINER()]
        = QList<NamespaceEntry>(getReadMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER()));
}

void DMetadataSettingsContainer::readOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container)
{
    KConfigGroup myItems = group.group(name);
    const auto list      = myItems.groupList();

    for (const QString& element : EXIV2_AS_CONST(list))
    {
        KConfigGroup gr      = myItems.group(element);
        NamespaceEntry ns;

        if (element.startsWith(QLatin1Char('#')))
        {
            ns.namespaceName = gr.readEntry("namespaceName");
        }
        else
        {
            ns.namespaceName = element;
        }

        ns.tagPaths          = (NamespaceEntry::TagType)gr.readEntry("tagPaths").toInt();
        ns.separator         = gr.readEntry("separator", QString());
        ns.nsType            = (NamespaceEntry::NamespaceType)gr.readEntry("nsType").toInt();
        ns.index             = gr.readEntry("index").toInt();
        ns.subspace          = (NamespaceEntry::NsSubspace)gr.readEntry("subspace").toInt();
        ns.alternativeName   = gr.readEntry("alternativeName", QString());
        ns.specialOpts       = (NamespaceEntry::SpecialOptions)gr.readEntry("specialOpts").toInt();
        ns.secondNameOpts    = (NamespaceEntry::SpecialOptions)gr.readEntry("secondNameOpts").toInt();
        ns.isDefault         = gr.readEntry(QLatin1String("isDefault"), QVariant(true)).toBool();
        ns.isDisabled        = gr.readEntry(QLatin1String("isDisabled"), QVariant(false)).toBool();
        QString conversion   = gr.readEntry("convertRatio", QString());

        if (!conversion.isEmpty() || (ns.nsType == NamespaceEntry::RATING))
        {
            const auto list2 = conversion.split(QLatin1String(","));

            for (const QString& str : EXIV2_AS_CONST(list2))
            {
                ns.convertRatio.append(str.toInt());
            }

            if (ns.convertRatio.size() != 6)
            {
                qCWarning(DIGIKAM_METAENGINE_LOG) << "Wrong size of rating conversion values in the config!";

                // fallback to default rating conversion values

                ns.convertRatio = QList<int>({0, 1, 2, 3, 4, 5});
            }
        }

        container.append(ns);
    }

    std::sort(container.begin(), container.end(), Digikam::s_dmcompare);
}

void DMetadataSettingsContainer::writeOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container) const
{
    KConfigGroup namespacesGroup = group.group(name);
    int index                    = 0;

    for (const NamespaceEntry& e : EXIV2_AS_CONST(container))
    {
        QString groupNumber = QString::fromLatin1("#%1")
                              .arg(index++, 4, 10, QLatin1Char('0'));

        KConfigGroup tmp = namespacesGroup.group(groupNumber);
        tmp.writeEntry("namespaceName",   e.namespaceName);

        if (!e.alternativeName.isEmpty())
        {
            tmp.writeEntry("alternativeName", e.alternativeName);
        }

        tmp.writeEntry("subspace",         (int)e.subspace);
        tmp.writeEntry("tagPaths",         (int)e.tagPaths);

        if (!e.separator.isEmpty())
        {
            tmp.writeEntry("separator",    e.separator);
        }

        tmp.writeEntry("nsType",           (int)e.nsType);

        if (!e.convertRatio.isEmpty())
        {
            tmp.writeEntry("convertRatio", e.convertRatio);
        }

        tmp.writeEntry("specialOpts",      (int)e.specialOpts);
        tmp.writeEntry("secondNameOpts",   (int)e.secondNameOpts);
        tmp.writeEntry("index",            e.index);
        tmp.writeEntry("isDisabled",       e.isDisabled);
        tmp.writeEntry("isDefault",        e.isDefault);
    }
}

QString DMetadataSettingsContainer::translateMappingKey(const QString& key) const
{

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    if      (NamespaceEntry::DM_TAG_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::TAGS].title.toString();
    }
    else if (NamespaceEntry::DM_TITLE_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::TITLE].title.toString();
    }
    else if (NamespaceEntry::DM_RATING_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::RATING].title.toString();
    }
    else if (NamespaceEntry::DM_COMMENT_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::COMMENT].title.toString();
    }
    else if (NamespaceEntry::DM_PICKLABEL_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::PICKLABEL].title.toString();
    }
    else if (NamespaceEntry::DM_COLORLABEL_CONTAINER() == key)
    {
        return namespaceTitleDefinitions[NamespaceEntry::COLORLABEL].title.toString();
    }

#else

    if      (NamespaceEntry::DM_TAG_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::TAGS].context,
                     namespaceTitleDefinitions[NamespaceEntry::TAGS].title);
    }
    else if (NamespaceEntry::DM_TITLE_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::TITLE].context,
                     namespaceTitleDefinitions[NamespaceEntry::TITLE].title);
    }
    else if (NamespaceEntry::DM_RATING_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::RATING].context,
                     namespaceTitleDefinitions[NamespaceEntry::RATING].title);
    }
    else if (NamespaceEntry::DM_COMMENT_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::COMMENT].context,
                     namespaceTitleDefinitions[NamespaceEntry::COMMENT].title);
    }
    else if (NamespaceEntry::DM_PICKLABEL_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::PICKLABEL].context,
                     namespaceTitleDefinitions[NamespaceEntry::PICKLABEL].title);
    }
    else if (NamespaceEntry::DM_COLORLABEL_CONTAINER() == key)
    {
        return i18nc(namespaceTitleDefinitions[NamespaceEntry::COLORLABEL].context,
                     namespaceTitleDefinitions[NamespaceEntry::COLORLABEL].title);
    }

#endif

    return key;
}

QDebug operator<<(QDebug dbg, const DMetadataSettingsContainer& inf)
{
    dbg.nospace() << "[DMetadataSettingsContainer] readMappings(";

    const auto keys = inf.mappingKeys();

    for (const QString& str : EXIV2_AS_CONST(keys))
    {
        dbg.nospace() << inf.getReadMapping(str) << "), ";
    }

    dbg.nospace() << "writeMappings(";

    const auto keys2 = inf.mappingKeys();

    for (const QString& str : EXIV2_AS_CONST(keys2))
    {
        dbg.nospace() << inf.getWriteMapping(str) << "), ";
    }

    dbg.nospace() << "unifyReadWrite("
                  << inf.unifyReadWrite() << ")";

    return dbg.space();
}

} // namespace Digikam
