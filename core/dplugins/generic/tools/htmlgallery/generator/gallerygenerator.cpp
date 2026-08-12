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

#include "gallerygenerator.h"

// Qt includes

#include <QDir>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QStringList>
#include <QtConcurrentMap>
#include <QApplication>
#include <QUrl>
#include <QList>
#include <QTemporaryFile>
#include <QSharedPointer>
#include <QEventLoop>

// KDE includes

#include <klocalizedstring.h>

// libxslt includes

#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xslt.h>
#include <libexslt/exslt.h>

// Local includes

#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "galleryelement.h"
#include "galleryelementfunctor.h"
#include "galleryinfo.h"
#include "gallerytheme.h"
#include "galleryxmlutils.h"
#include "htmlwizard.h"
#include "dfileoperations.h"

namespace DigikamGenericHtmlGalleryPlugin
{

typedef QMap<QByteArray, QByteArray> XsltParameterMap;

class Q_DECL_HIDDEN GalleryGenerator::Private
{
public:

    // Url => local temp path
    typedef QHash<QUrl, QString> RemoteUrlHash;

public:

    Private() = default;

public:

    GalleryGenerator*           that        = nullptr;
    GalleryInfo*                info        = nullptr;
    GalleryTheme::Ptr           theme;

    // State info
    bool                        warnings    = false;
    QString                     xmlFileName;

    bool                        cancel      = false;
    DHistoryView*               pview       = nullptr;
    DProgressWdg*               pbar        = nullptr;

    QEventLoop*                 waitLoop    = nullptr;

    QSharedPointer<const char*> params;

public:

    bool init()
    {
        cancel = false;
        theme  = GalleryTheme::findByInternalName(info->theme());

        if (!theme)
        {
            logError(i18nc("@info", "Could not find theme in '%1'", info->theme()) );
            return false;
        }

        pview->setVisible(true);
        pbar->setVisible(true);

        return true;
    }

    bool createDir(const QString& dirName)
    {
        logInfo(i18nc("@info", "Create directories"));

        if (!QDir().mkpath(dirName))
        {
            logError(i18nc("@info", "Could not create folder '%1'", QDir::toNativeSeparators(dirName)));
            return false;
        }

        return true;
    }

    bool copyTheme()
    {
        logInfo(i18nc("@info", "Copying theme"));

        QUrl srcUrl  = QUrl::fromLocalFile(theme->directory());
        QUrl destUrl = info->destUrl().adjusted(QUrl::StripTrailingSlash);
        QDir themeDir(destUrl.toLocalFile() + QLatin1Char('/') + srcUrl.fileName());

        if (themeDir.exists())
        {
            themeDir.removeRecursively();
        }

        bool ok = DFileOperations::copyFolderRecursively(srcUrl.toLocalFile(), destUrl.toLocalFile());

        if (!ok)
        {
            logError(i18nc("@info", "Could not copy theme"));
            return false;
        }

        return true;
    }

    bool generateImagesAndXML()
    {
        logInfo(i18nc("@info", "Generate images and XML files"));
        QString baseDestDir = info->destUrl().toLocalFile();

        if (!createDir(baseDestDir))
        {
            return false;
        }

        xmlFileName         = baseDestDir + QLatin1String("/gallery.xml");
        XMLWriter xmlWriter;

        if (!xmlWriter.open(xmlFileName))
        {
            logError(i18nc("@info", "Could not create gallery.xml"));
            return false;
        }

        XMLElement collectionsX(xmlWriter, QLatin1String("collections"));

        if (info->m_getOption == GalleryInfo::ALBUMS)
        {
            // use QMultiMap to sorting albums

            QMultiMap<QString, QPair<int, QString> > albumMap;
            DInfoInterface::DAlbumIDs::ConstIterator albumIt  = info->m_albumList.constBegin();
            DInfoInterface::DAlbumIDs::ConstIterator albumEnd = info->m_albumList.constEnd();

            for ( ; albumIt != albumEnd ; ++albumIt)
            {
                int id = *albumIt;
                DInfoInterface::DInfoMap inf;

                if (info->m_iface)
                {
                    inf = info->m_iface->albumInfo(id);
                }

                DAlbumInfo anf(inf);
                albumMap.insert(anf.title(), qMakePair(id, anf.caption()));
            }

            // Loop over albums selection

            QMultiMap<QString, QPair<int, QString> >::const_iterator mapIt  = albumMap.constBegin();
            QMultiMap<QString, QPair<int, QString> >::const_iterator mapEnd = albumMap.constEnd();

            for ( ; mapIt != mapEnd ; ++mapIt)
            {
                int id                     = mapIt.value().first;
                QString title              = mapIt.key();
                QString collectionFileName = webifyFileName(title);
                QString destDir            = baseDestDir + QLatin1Char('/') + collectionFileName;

                if (!createDir(destDir))
                {
                    return false;
                }

                XMLElement collectionX(xmlWriter,  QLatin1String("collection"));
                xmlWriter.writeElement("name",     title);
                xmlWriter.writeElement("fileName", collectionFileName);
                xmlWriter.writeElement("comment",  mapIt.value().second);

                // Gather image element list

                QList<QUrl> imageList;

                if (info->m_iface)
                {
                    imageList = info->m_iface->albumsItems(DInfoInterface::DAlbumIDs() << id);
                }

                if (!processImages(xmlWriter, imageList, title, destDir))
                {
                    return false;
                }
            }
        }
        else
        {
            QString title              = info->imageSelectionTitle();
            QString collectionFileName = webifyFileName(title);
            QString destDir            = baseDestDir + QLatin1Char('/') + collectionFileName;

            if (!createDir(destDir))
            {
                return false;
            }

            XMLElement collectionX(xmlWriter,  QLatin1String("collection"));
            xmlWriter.writeElement("name",     title);
            xmlWriter.writeElement("fileName", collectionFileName);

            if (!processImages(xmlWriter, info->m_imageList, title, destDir))
            {
                return false;
            }
        }

        return true;
    }

    bool processImages(XMLWriter& xmlWriter, const QList<QUrl>& imageList,
                       const QString& title, const QString& destDir)
    {
        RemoteUrlHash remoteUrlHash;

        if (!downloadRemoteUrls(title, imageList, &remoteUrlHash))
        {
            return false;
        }

        QList<GalleryElement> imageElementList;

        for (const QUrl& url : std::as_const(imageList))
        {
            const QString path = remoteUrlHash.value(url, url.toLocalFile());

            if (path.isEmpty())
            {
                continue;
            }

            DInfoInterface::DInfoMap inf;

            if (info->m_iface)
            {
                inf = info->m_iface->itemInfo(url);
            }

            GalleryElement element = GalleryElement(inf);
            element.m_path         = remoteUrlHash.value(url, url.toLocalFile());
            imageElementList << element;
        }

        // Generate images

        pbar->setMaximum(imageElementList.count());
        logInfo(i18nc("@info", "Generating files for \"%1\"", title));
        GalleryElementFunctor functor(that, info, destDir);
        QFuture<void> future = QtConcurrent::map(imageElementList, functor);
        QFutureWatcher<void> watcher;

        connect(&watcher, SIGNAL(progressValueChanged(int)),
                pbar, SLOT(setValue(int)),
                Qt::QueuedConnection);

        connect(&watcher, SIGNAL(finished()),
                waitLoop, SLOT(quit()),
                Qt::QueuedConnection);

        watcher.setFuture(future);

        waitLoop->exec();

        if (!future.isFinished())
        {
            future.cancel();
            future.waitForFinished();

            return false;
        }

        if (cancel)
        {
            return false;
        }

        // Generate xml

        for (const GalleryElement& element : std::as_const(imageElementList))
        {
            element.appendToXML(xmlWriter, info->copyOriginalImage());
        }

        return true;
    }

    bool generateHTML()
    {
        logInfo(i18nc("@info", "Generating HTML files"));

        QString xsltFileName                                 = theme->directory() + QLatin1String("/template.xsl");
        CWrapper<xsltStylesheetPtr, xsltFreeStylesheet> xslt = xsltParseStylesheetFile(
            reinterpret_cast<const xmlChar*>(QDir::toNativeSeparators(xsltFileName).toUtf8().data()));

        if (!xslt)
        {
            logError(i18nc("@info", "Could not load XSL file '%1'", xsltFileName));
            return false;
        }

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlGallery =
            xmlParseFile(QDir::toNativeSeparators(xmlFileName).toUtf8().data() );

        if (!xmlGallery)
        {
            logError(i18nc("@info", "Could not load XML file '%1'", xmlFileName));
            return false;
        }

        // Prepare parameters

        XsltParameterMap map;
        addI18nParameters(map);
        addThemeParameters(map);

        const char** data              = new const char*[map.size() * 2 + 1];
        params                         = QSharedPointer<const char*>(data);
        XsltParameterMap::Iterator it  = map.begin();
        XsltParameterMap::Iterator end = map.end();
        const char** ptr               = params.data();

        for ( ; it != end ; ++it)
        {
            *ptr = it.key().data();
            ++ptr;
            *ptr = it.value().data();
            ++ptr;
        }

        *ptr = nullptr;

        // Move to the destination dir, so that external documents get correctly produced.

        QString oldCD                             = QDir::currentPath();
        QDir::setCurrent(info->destUrl().toLocalFile());

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlOutput = xsltApplyStylesheet(xslt, xmlGallery, params.data());

        QDir::setCurrent(oldCD);

        if (!xmlOutput)
        {
            logError(i18nc("@info", "Error processing XML file"));
            return false;
        }

        QString destFileName = QDir::toNativeSeparators(info->destUrl().toLocalFile() +
                                                        QLatin1String("/index.html"));

        if (xsltSaveResultToFilename(destFileName.toUtf8().data(), xmlOutput, xslt, 0) == -1)
        {
            logError(i18nc("@info", "Could not open '%1' for writing", destFileName));
            return false;
        }

        return true;
    }

    bool downloadRemoteUrls(const QString& collectionName,
                            const QList<QUrl>& _list,
                            RemoteUrlHash* const hash)
    {
        Q_ASSERT(hash);

        QList<QUrl> list;

        for (const QUrl& url : std::as_const(_list))
        {
            if (!url.isLocalFile())
            {
                list << url;
            }
        }

        if (list.count() == 0)
        {
            return true;
        }

        logInfo(i18nc("@info", "Downloading remote files for \"%1\"", collectionName));

        pbar->setMaximum(list.count());
        int count = 0;

        for (const QUrl& url : std::as_const(list))
        {
            if (cancel)
            {
                return false;
            }

            QTemporaryFile tempFile;
            tempFile.setFileTemplate(QLatin1String("htmlgallery-"));

            if (!tempFile.open())
            {
                logError(i18nc("@info", "Could not open temporary file"));
                return false;
            }

            QTemporaryFile tempPath;
            tempPath.setFileTemplate(tempFile.fileName());
            tempPath.setAutoRemove(false);

            if (tempPath.open() &&
                DFileOperations::copyFiles(QStringList() << url.toLocalFile(), tempPath.fileName()))
            {
                hash->insert(url, tempFile.fileName());
            }
            else
            {
                logWarning(i18nc("@info", "Could not download %1", url.toDisplayString()));
                hash->insert(url, QString());
            }

            tempPath.close();
            tempFile.close();

            ++count;
            pbar->setValue(count);
        }

        return true;
    }

    /**
     * Add to map all the i18n parameters.
     */
    void addI18nParameters(XsltParameterMap& map)
    {
        map["i18nPrevious"]                   = makeXsltParam(i18nc("@info: xslt param", "Previous"));
        map["i18nNext"]                       = makeXsltParam(i18nc("@info: xslt param", "Next"));
        map["i18nCollectionList"]             = makeXsltParam(i18nc("@info: xslt param", "Album List"));
        map["i18nOriginalImage"]              = makeXsltParam(i18nc("@info: xslt param", "Original Image"));
        map["i18nUp"]                         = makeXsltParam(i18nc("@info: xslt param", "Go Up"));
        // Exif Tag
        map["i18nexifimagemake"]              = makeXsltParam(i18nc("@info: xslt param", "Make"));
        map["i18nexifimagemodel"]             = makeXsltParam(i18nc("@info: Camera Model", "Model"));
        map["i18nexifimageorientation"]       = makeXsltParam(i18nc("@info: xslt param", "Image Orientation"));
        map["i18nexifimagexresolution"]       = makeXsltParam(i18nc("@info: xslt param", "Image X Resolution"));
        map["i18nexifimageyresolution"]       = makeXsltParam(i18nc("@info: xslt param", "Image Y Resolution"));
        map["i18nexifimageresolutionunit"]    = makeXsltParam(i18nc("@info: xslt param", "Image Resolution Unit"));
        map["i18nexifimagedatetime"]          = makeXsltParam(i18nc("@info: xslt param", "Image Date Time"));
        map["i18nexifimageycbcrpositioning"]  = makeXsltParam(i18nc("@info: xslt param", "YCBCR Positioning"));
        map["i18nexifphotoexposuretime"]      = makeXsltParam(i18nc("@info: xslt param", "Exposure Time"));
        map["i18nexifphotofnumber"]           = makeXsltParam(i18nc("@info: xslt param", "F Number"));
        map["i18nexifphotoexposureprogram"]   = makeXsltParam(i18nc("@info: xslt param", "Exposure Index"));
        map["i18nexifphotoisospeedratings"]   = makeXsltParam(i18nc("@info: xslt param", "ISO Speed Ratings"));
        map["i18nexifphotoshutterspeedvalue"] = makeXsltParam(i18nc("@info: xslt param", "Shutter Speed Value"));
        map["i18nexifphotoaperturevalue"]     = makeXsltParam(i18nc("@info: xslt param", "Aperture Value"));
        map["i18nexifphotofocallength"]       = makeXsltParam(i18nc("@info: xslt param", "Focal Length"));
        map["i18nexifgpsaltitude"]            = makeXsltParam(i18nc("@info: xslt param", "GPS Altitude"));
        map["i18nexifgpslatitude"]            = makeXsltParam(i18nc("@info: xslt param", "GPS Latitude"));
        map["i18nexifgpslongitude"]           = makeXsltParam(i18nc("@info: xslt param", "GPS Longitude"));
    }

    /**
     * Add to map all the theme parameters, as specified by the user.
     */
    void addThemeParameters(XsltParameterMap& map)
    {
        GalleryTheme::ParameterList parameterList      = theme->parameterList();
        QString themeInternalName                      = theme->internalName();
        GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
        GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

        for ( ; it != end ; ++it)
        {
            const AbstractThemeParameter* const themeParameter = *it;
            QByteArray internalName                            = themeParameter->internalName();
            QString value                                      = info->getThemeParameterValue(themeInternalName,
                                                                                              QLatin1String(internalName),
                                                                                              themeParameter->defaultValue());

            map[internalName]                                  = makeXsltParam(value);
        }
    }

    /**
     * Prepare an XSLT param, managing quote mess.
     * abc   => 'abc'
     * a"bc  => 'a"bc'
     * a'bc  => "a'bc"
     * a"'bc => concat('a"', "'", 'bc')
     */
    QByteArray makeXsltParam(const QString& txt)
    {
        QString param;
        static const char apos  = '\'';
        static const char quote = '"';

        if      (txt.indexOf(QLatin1Char(apos)) == -1)
        {
            // First or second case: no apos
            param = QLatin1Char(apos) + txt + QLatin1Char(apos);
        }
        else if (txt.indexOf(QLatin1Char(quote)) == -1)
        {
            // Third case: only apos, no quote
            param = QLatin1Char(quote) + txt + QLatin1Char(quote);
        }
        else
        {
            // Forth case: both apos and quote :-(

            const QStringList lst = txt.split(QLatin1Char(apos), Qt::KeepEmptyParts);

            QStringList::ConstIterator it  = lst.constBegin();
            QStringList::ConstIterator end = lst.constEnd();
            param                          = QLatin1String("concat(");
            param                         += QLatin1Char(apos) + *it + QLatin1Char(apos);
            ++it;

            for ( ; it != end ; ++it)
            {
                param += QLatin1String(", \"'\", ");
                param += QLatin1Char(apos) + *it + QLatin1Char(apos);
            }

            param += QLatin1Char(')');
        }
/*
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "param: " << txt << " => " << param;
*/
        return param.toUtf8();
    }

    void logInfo(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ProgressEntry);
    }

    void logError(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ErrorEntry);
    }

    void logWarning(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::WarningEntry);
        warnings = true;
    }
};

// ----------------------------------------------------------------------

GalleryGenerator::GalleryGenerator(GalleryInfo* const info)
    : QObject(),
      d      (new Private)
{
    d->that     = this;
    d->info     = info;
    d->warnings = false;

    d->waitLoop = new QEventLoop(this);

    connect(this, SIGNAL(logWarningRequested(QString)),
            SLOT(logWarning(QString)), Qt::QueuedConnection);
}

GalleryGenerator::~GalleryGenerator()
{
    delete d;
}

bool GalleryGenerator::run()
{
    if (!d->init())
    {
        return false;
    }

    QString destDir = d->info->destUrl().toLocalFile();
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << destDir;

    if (!d->createDir(destDir))
    {
        return false;
    }

    if (!d->copyTheme())
    {
        return false;
    }

    if (!d->generateImagesAndXML())
    {
        return false;
    }

    exsltRegisterAll();

    bool result = d->generateHTML();

    xsltCleanupGlobals();
    xmlCleanupParser();

    return result;
}

bool GalleryGenerator::warnings() const
{
    return d->warnings;
}

void GalleryGenerator::logWarning(const QString& text)
{
    d->logWarning(text);
}

void GalleryGenerator::slotCancel()
{
    d->cancel = true;
    d->waitLoop->quit();
}

void GalleryGenerator::setProgressWidgets(DHistoryView* const pView, DProgressWdg* const pBar)
{
    d->pview = pView;
    d->pbar  = pBar;
/*
    connect(d->pbar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancel()));
*/
}

QString GalleryGenerator::webifyFileName(const QString& fname)
{
    QString fileName = fname.toLower();

    // Remove potentially troublesome chars

    return fileName.replace(QRegularExpression(QLatin1String("[^-0-9a-z]+")), QLatin1String("_"));
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_gallerygenerator.cpp"
