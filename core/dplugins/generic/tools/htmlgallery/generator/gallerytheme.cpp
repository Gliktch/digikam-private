/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gallerytheme.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QDir>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#   include <QTextCodec>

#endif

// KDE includes

#include <kconfiggroup.h>
#include <kdesktopfile.h>

// Local includes

#include "digikam_debug.h"
#include "colorthemeparameter.h"
#include "intthemeparameter.h"
#include "listthemeparameter.h"
#include "stringthemeparameter.h"
#include "captionthemeparameter.h"

namespace DigikamGenericHtmlGalleryPlugin
{

static const QLatin1String AUTHOR_GROUP           ("X-HTMLGallery Author");
static const QLatin1String PARAMETER_GROUP_PREFIX ("X-HTMLGallery Parameter ");
static const QLatin1String PARAMETER_TYPE_KEY     ("Type");
static const QLatin1String PREVIEW_GROUP          ("X-HTMLGallery Preview");
static const QLatin1String OPTIONS_GROUP          ("X-HTMLGallery Options");
static const QLatin1String CAPTION_PARAMETER_TYPE ("caption");
static const QLatin1String STRING_PARAMETER_TYPE  ("string");
static const QLatin1String LIST_PARAMETER_TYPE    ("list");
static const QLatin1String COLOR_PARAMETER_TYPE   ("color");
static const QLatin1String INT_PARAMETER_TYPE     ("int");

static GalleryTheme::List sList;

class Q_DECL_HIDDEN GalleryTheme::Private
{
public:

    Private() = default;

    KDesktopFile* desktopFile = nullptr;
    QUrl          url;
    ParameterList parameterList;

public:

    /**
     * Return the list of parameters defined in the desktop file. We need to
     * parse the file ourselves to preserve parameter order.
     */
    QStringList readParameterNameList(const QString& desktopFileName)
    {
        QStringList list;
        QFile file(desktopFileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            return QStringList();
        }

        QTextStream stream(&file);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        stream.setEncoding(QStringConverter::Utf8);

#else

        stream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

        QString prefix = QLatin1String("[") + QLatin1String(PARAMETER_GROUP_PREFIX);

        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            line         = line.trimmed();

            if (!line.startsWith(prefix))
            {
                continue;
            }

            // Remove opening bracket and group prefix

            line         = line.mid(prefix.length());

            // Remove closing bracket

            line.truncate(line.length() - 1);

            list.append(line);
        }

        file.close();

        return list;
    }

    void init(const QString& desktopFileName)
    {
        delete desktopFile;

        desktopFile                   = new KDesktopFile(desktopFileName);
        url                           = QUrl::fromLocalFile(desktopFileName);
        QStringList parameterNameList = readParameterNameList(desktopFileName);

        readParameters(parameterNameList);
    }

    void readParameters(const QStringList& list)
    {
        QStringList::ConstIterator it  = list.constBegin();
        QStringList::ConstIterator end = list.constEnd();

        for ( ; it != end ; ++it)
        {
            QString groupName                 = QLatin1String(PARAMETER_GROUP_PREFIX) + *it;
            QByteArray internalName           = it->toUtf8();
            KConfigGroup group                = desktopFile->group(groupName);
            QString type                      = group.readEntry(PARAMETER_TYPE_KEY);
            AbstractThemeParameter* parameter = nullptr;

            if      (type == QLatin1String(STRING_PARAMETER_TYPE))
            {
                parameter = new StringThemeParameter();
            }
            else if (type == QLatin1String(CAPTION_PARAMETER_TYPE))
            {
                parameter = new CaptionThemeParameter();
            }
            else if (type == QLatin1String(LIST_PARAMETER_TYPE))
            {
                parameter = new ListThemeParameter();
            }
            else if (type == QLatin1String(COLOR_PARAMETER_TYPE))
            {
                parameter = new ColorThemeParameter();
            }
            else if (type == QLatin1String(INT_PARAMETER_TYPE))
            {
                parameter = new IntThemeParameter();
            }
            else
            {
                qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Parameter '" << internalName
                                                       << "' has unknown type '" << type
                                                       << "'. Falling back to string type\n";
                parameter = new StringThemeParameter();
            }

            parameter->init(internalName, &group);
            parameterList << parameter;
        }
    }
};

GalleryTheme::GalleryTheme()
   : d(new Private)
{
}

GalleryTheme::~GalleryTheme()
{
    delete d->desktopFile;
    delete d;
}

const GalleryTheme::List& GalleryTheme::getList()
{
    if (sList.isEmpty())
    {
        QStringList list;
        QStringList internalNameList;
        const QStringList filter     = QStringList() << QLatin1String("*.desktop");
        const QStringList themesDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                                 QLatin1String("digikam/themes"),
                                                                 QStandardPaths::LocateDirectory);

        for (const QString& themeDir : std::as_const(themesDirs))
        {
            const auto infs = QDir(themeDir).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

            for (const QFileInfo& themeInfo : infs)
            {
                const auto files = QDir(themeInfo.absoluteFilePath()).entryInfoList(filter);

                for (const QFileInfo& deskFile : files)
                {
                    list << deskFile.absoluteFilePath();
                }
            }
        }

        QStringList::ConstIterator it  = list.constBegin();
        QStringList::ConstIterator end = list.constEnd();

        for ( ; it != end ; ++it)
        {
            GalleryTheme::Ptr theme(new GalleryTheme);
            theme->d->init(*it);
            QString intName = theme->internalName();

            if (!internalNameList.contains(intName))
            {
                sList << theme;
                internalNameList << intName;
            }
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "HTML Gallery Themes found:" << sList.size();

    return sList;
}

GalleryTheme::Ptr GalleryTheme::findByInternalName(const QString& internalName)
{
    const GalleryTheme::List& lst         = getList();
    GalleryTheme::List::ConstIterator it  = lst.constBegin();
    GalleryTheme::List::ConstIterator end = lst.constEnd();

    for ( ; it != end ; ++it)
    {
        GalleryTheme::Ptr theme = *it;

        if (theme->internalName() == internalName)
        {
            return theme;
        }
    }

    return GalleryTheme::Ptr(nullptr);
}

QString GalleryTheme::internalName() const
{
    return d->url.fileName();
}

QString GalleryTheme::name() const
{
    return d->desktopFile->readName();
}

QString GalleryTheme::comment() const
{
    return d->desktopFile->readComment();
}

QString GalleryTheme::directory() const
{
    return d->url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();
}

QString GalleryTheme::authorName() const
{
    return d->desktopFile->group(AUTHOR_GROUP).readEntry("Name");
}

QString GalleryTheme::authorUrl() const
{
    return d->desktopFile->group(AUTHOR_GROUP).readEntry("Url");
}

QString GalleryTheme::previewName() const
{
    return d->desktopFile->group(PREVIEW_GROUP).readEntry("Name");
}

QString GalleryTheme::previewUrl() const
{
    return d->desktopFile->group(PREVIEW_GROUP).readEntry("Url");
}

bool GalleryTheme::allowNonsquareThumbnails() const
{
    return d->desktopFile->group(OPTIONS_GROUP).readEntry("Allow-non-square-thumbnails", false);
}

GalleryTheme::ParameterList GalleryTheme::parameterList() const
{
    return d->parameterList;
}

} // namespace DigikamGenericHtmlGalleryPlugin
