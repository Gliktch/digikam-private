/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "copyfilestask.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"
#include "panomanager.h"

namespace DigikamGenericPanoramaPlugin
{

CopyFilesTask::CopyFilesTask(const QString& workDirPath,
                             const QUrl& panoUrl,
                             const QUrl& finalPanoUrl,
                             const QUrl& ptoUrl,
                             const PanoramaItemUrlsMap& urls,
                             bool sPTO,
                             bool GPlusMetadata)
    : PanoTask        (PANO_COPY, workDirPath),
      panoUrl         (panoUrl),
      finalPanoUrl    (finalPanoUrl),
      ptoUrl          (ptoUrl),
      urlList         (&urls),
      savePTO         (sPTO),
      addGPlusMetadata(GPlusMetadata)
{
}

void CopyFilesTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    QFile     panoFile(panoUrl.toLocalFile());
    QFile     finalPanoFile(finalPanoUrl.toLocalFile());

    QFileInfo fi(QDir::toNativeSeparators(finalPanoUrl.toLocalFile()));
    QUrl      finalPTOUrl = finalPanoUrl.adjusted(QUrl::RemoveFilename);
    finalPTOUrl.setPath(finalPTOUrl.path() + fi.completeBaseName() + QLatin1String(".pto"));

    QFile     ptoFile(ptoUrl.toLocalFile());
    QFile     finalPTOFile(finalPTOUrl.toLocalFile());

    if (!panoFile.exists())
    {
        errString = i18n("Temporary panorama file does not exist.");
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Temporary panorama file does not exist: " << panoUrl;
        successFlag = false;
        return;
    }

    if (finalPanoFile.exists())
    {
        errString = i18n("A panorama file named <filename>%1</filename> already exists.", finalPanoUrl.fileName());
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Final panorama file already exists: " << finalPanoUrl;
        successFlag = false;
        return;
    }

    if (savePTO && !ptoFile.exists())
    {
        errString = i18n("Temporary project file does not exist.");
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Temporary project file does not exist: " << ptoUrl;
        successFlag = false;
        return;
    }

    if (savePTO && finalPTOFile.exists())
    {
        errString = i18n("A project file named <filename>%1</filename> already exist.", finalPTOUrl.fileName());
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Final project file already exist: " << finalPTOUrl;
        successFlag = false;
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Copying GPS info...";

    // Find first src image which contain geolocation and save it to target pano file.

    double lat, lng, alt;
    QScopedPointer<DMetadata> meta(new DMetadata);

    for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin() ; i != urlList->constEnd() ; ++i)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << i.key();

        meta->load(i.key().toLocalFile());

        if (meta->getGPSInfo(alt, lat, lng))
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "GPS info found and saved in " << panoUrl;
            meta->load(panoUrl.toLocalFile());
            meta->setGPSInfo(alt, lat, lng);
            meta->applyChanges(true);
            break;
        }
    }

    // Restore usual and common metadata from first shot.

    meta->load(urlList->constBegin().key().toLocalFile());
    QByteArray iptc = meta->getIptc();
    QByteArray xmp  = meta->getXmp();
    QString make    = meta->getExifTagString("Exif.Image.Make");
    QString model   = meta->getExifTagString("Exif.Image.Model");
    QDateTime dt    = meta->getItemDateTime();

    meta->load(panoUrl.toLocalFile());
    meta->setIptc(iptc);
    meta->setXmp(xmp);
    meta->setXmpTagString("Xmp.tiff.Make",   make);
    meta->setXmpTagString("Xmp.tiff.Model", model);
    meta->setImageDateTime(dt);

    QString filesList;

    for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin() ; i != urlList->constEnd() ; ++i)
    {
        filesList.append(i.key().fileName() + QLatin1String(" ; "));
    }

    filesList.truncate(filesList.length()-3);

    meta->setXmpTagString("Xmp.digiKam.PanoramaInputFiles", filesList);

    // NOTE : See https://developers.google.com/photo-sphere/metadata/ for details

    if (addGPlusMetadata)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Adding PhotoSphere metadata...";
        meta->registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/panorama/"),      // krazy:exclude=insecurenet
                                   QLatin1String("GPano"));
        meta->setXmpTagString("Xmp.GPano.UsePanoramaViewer", QLatin1String("True"));
        meta->setXmpTagString("Xmp.GPano.StitchingSoftware", QLatin1String("Panorama digiKam tool with Hugin"));
        meta->setXmpTagString("Xmp.GPano.ProjectionType",    QLatin1String("equirectangular"));
    }

    meta->applyChanges(true);

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Copying panorama file...";

    if (!panoFile.copy(finalPanoUrl.toLocalFile()) || !panoFile.remove())
    {
        errString   = i18n("Cannot move panorama from <filename>%1</filename> to <filename>%2</filename>.",
                           panoUrl.toLocalFile(),
                           finalPanoUrl.toLocalFile());
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Cannot move panorama: QFile error = " << panoFile.error();
        successFlag = false;

        return;
    }

    if (savePTO)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Copying project file...";

        if (!ptoFile.copy(finalPTOUrl.toLocalFile()))
        {
            errString   = i18n("Cannot move project file from <filename>%1</filename> to <filename>%2</filename>.",
                               panoUrl.toLocalFile(),
                               finalPanoUrl.toLocalFile());
            successFlag = false;

            return;
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Copying converted RAW files...";

        for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin() ; i != urlList->constEnd() ; ++i)
        {
            if (DRawDecoder::isRawFile(i.key()))
            {
                QUrl finalImgUrl = finalPanoUrl.adjusted(QUrl::RemoveFilename);
                finalImgUrl.setPath(finalImgUrl.path() + i->preprocessedUrl.fileName());
                QFile finalImgFile(finalImgUrl.toLocalFile());
                QFile imgFile(i->preprocessedUrl.toLocalFile());

                if (finalImgFile.exists())
                {
                    continue;
                }

                if (!imgFile.copy(finalImgUrl.toLocalFile()))
                {
                    errString   = i18n("Cannot copy converted image file from <filename>%1</filename> to <filename>%2</filename>.",
                                       i->preprocessedUrl.toLocalFile(),
                                       finalImgUrl.toLocalFile());
                    successFlag = false;

                    return;
                }
            }
        }
    }

    Q_EMIT PanoManager::instance()->updateHostApp(finalPanoUrl);

    successFlag = true;
}

} // namespace DigikamGenericPanoramaPlugin
