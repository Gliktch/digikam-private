/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensfuniface.h"

// Qt includes

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

// Local includes

#include "digikam_debug.h"

// Disable deprecated API from older Lensfun.

#if !LENSFUN_TEST_VERSION(0,3,99)

#   if defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   endif

#endif

namespace Digikam
{

class Q_DECL_HIDDEN LensFunIface::Private
{
public:

    Private() = default;

    QString  metadataMatchDebugStr(MetadataMatch val)          const;
    LensList findLenses(const lfCamera* const camera,
                        const QString& lensDesc,
                        const QString& lensMaker = QString())  const;

    double checkSimilarity(const QString& a, const QString& b) const;

public:

    // To be used for modification

    LensFunContainer       settings;

    // Database items

    lfDatabase*            lfDb         = nullptr;
    const lfCamera* const* lfCameras    = nullptr;

    QString                makeDescription;
    QString                modelDescription;
    QString                lensDescription;

    LensPtr                usedLens     = nullptr;
    DevicePtr              usedCamera   = nullptr;
};

QString LensFunIface::Private::metadataMatchDebugStr(MetadataMatch val) const
{
    QString ret;

    switch (val)
    {
        case MetadataNoMatch:
        {
            ret = QLatin1String("No Match");
            break;
        }

        case MetadataPartialMatch:
        {
            ret = QLatin1String("Partial Match");
            break;
        }

        default:
        {
            ret = QLatin1String("Exact Match");
            break;
        }
    }

    return ret;
}

LensFunIface::LensList LensFunIface::Private::findLenses(const lfCamera* const lfCamera,
                                                         const QString& lensDesc,
                                                         const QString& lensMaker) const
{
    LensList lensList;

    if (lfCamera)
    {
        const char* const maker     = lensMaker.isEmpty() ? nullptr : lensMaker.toLatin1().constData();
        const char* const model     = lensDesc.isEmpty()  ? nullptr : lensDesc.toLatin1().constData();
        const lfLens* const *lfLens = lfDb->FindLenses(lfCamera, maker, model);

        while (lfLens && *lfLens)
        {
            lensList << (*lfLens);
            ++lfLens;
        }
    }

    return lensList;
}

// Inspired by https://www.qtcentre.org/threads/49601-String-similarity-check

double LensFunIface::Private::checkSimilarity(const QString& a, const QString& b) const
{
    if (a.isEmpty() || b.isEmpty())
    {
        return 0.0;
    }

    const int chars = 3;
    int counter     = 0;
    QString spaces  = QString::fromLatin1(" ").repeated(chars - 1);
    QString aa      = spaces + a + spaces;
    QString bb      = spaces + b + spaces;

    for (int i = 0 ; i < (aa.size() - (chars - 1)) ; ++i)
    {
        QString part = aa.mid(i, chars);

        if (bb.contains(part, Qt::CaseInsensitive))
        {
            ++counter;
        }
    }

    QString s = (aa.length() < bb.length()) ? aa : bb;

    return (100.0 * counter / (s.length() - (chars - 1)));
}

// -------------------------------------------------------------------------------

LensFunIface::LensFunIface()
    : d(new Private)
{
#if LENSFUN_TEST_VERSION(0,3,99)

    d->lfDb          = lf_db_create();

#else

    d->lfDb          = lf_db_new();

#endif

   QString lensPath;
   QDir    lensDir;

#if LENSFUN_TEST_VERSION(0,3,99)

    // Use last Lensfun version data dir.

    lensPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                      QLatin1String("lensfun/updates/version_2"),
                                      QStandardPaths::LocateDirectory);

    if (lensPath.isEmpty())
    {
        lensPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                          QLatin1String("lensfun/version_2"),
                                          QStandardPaths::LocateDirectory);
    }

#else

    // Use Lensfun version revision 1.

    lensPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                      QLatin1String("lensfun/updates/version_1"),
                                      QStandardPaths::LocateDirectory);

    if (lensPath.isEmpty())
    {
        lensPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                          QLatin1String("lensfun/version_1"),
                                          QStandardPaths::LocateDirectory);
    }

#endif

    lensDir = QDir(lensPath, QLatin1String("*.xml"));

    if (lensDir.entryList().isEmpty())
    {
        // Fail-back to revision 0 which host XML data in root data directory.

        lensPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                          QLatin1String("lensfun"),
                                          QStandardPaths::LocateDirectory);

        lensDir  = QDir(lensPath, QLatin1String("*.xml"));
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Using root lens database dir:" << lensPath;
    const auto lenses = lensDir.entryList();

    for (const QString& lens : lenses)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Load lens database file: " << lens;

        d->lfDb->Load(QFile::encodeName(lensDir.absoluteFilePath(lens)).constData());
    }

    d->lfDb->Load();

    d->lfCameras = d->lfDb->GetCameras();
}

LensFunIface::~LensFunIface()
{
    lf_db_destroy(d->lfDb);
    delete d;
}

void LensFunIface::setSettings(const LensFunContainer& other)
{
    d->settings   = other;
    d->usedCamera = findCamera(d->settings.cameraMake, d->settings.cameraModel);
    d->usedLens   = findLens(d->settings.lensModel);
}

LensFunContainer LensFunIface::settings() const
{
    return d->settings;
}

LensFunIface::DevicePtr LensFunIface::usedCamera() const
{
    return d->usedCamera;
}

void LensFunIface::setUsedCamera(DevicePtr cam)
{
    d->usedCamera           = cam;
    d->settings.cameraMake  = d->usedCamera ? QLatin1String(d->usedCamera->Maker) : QString();
    d->settings.cameraModel = d->usedCamera ? QLatin1String(d->usedCamera->Model) : QString();
    d->settings.cropFactor  = d->usedCamera ? d->usedCamera->CropFactor           : -1.0;
}

LensFunIface::LensPtr LensFunIface::usedLens() const
{
    return d->usedLens;
}

void LensFunIface::setUsedLens(LensPtr lens)
{
    d->usedLens            = lens;
    d->settings.lensModel  = d->usedLens ? QLatin1String(d->usedLens->Model)
                                         : QString();
}

lfDatabase* LensFunIface::lensFunDataBase() const
{
    return d->lfDb;
}

QString LensFunIface::makeDescription() const
{
    return d->makeDescription;
}

QString LensFunIface::modelDescription() const
{
    return d->modelDescription;
}

QString LensFunIface::lensDescription() const
{
    return d->lensDescription;
}

const lfCamera* const* LensFunIface::lensFunCameras() const
{
    return d->lfCameras;
}

void LensFunIface::setFilterSettings(const LensFunContainer& other)
{
    d->settings.filterCCA = other.filterCCA;
    d->settings.filterVIG = other.filterVIG;
    d->settings.filterDST = other.filterDST;
    d->settings.filterGEO = other.filterGEO;
}

LensFunIface::DevicePtr LensFunIface::findCamera(const QString& make, const QString& model) const
{
    const lfCamera* const* cameras = d->lfDb->GetCameras();

    while (cameras && *cameras)
    {
        DevicePtr cam = *cameras;
/*
        qCDebug(DIGIKAM_DIMG_LOG) << "Query camera:" << cam->Maker << "-" << cam->Model;
*/
        if (
            (QString::fromLatin1(cam->Maker).toLower() == make.toLower()) &&
            (QString::fromLatin1(cam->Model).toLower() == model.toLower())
           )
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Search for camera "
                                      << make << "-"
                                      << model << " ==> true";
            return cam;
        }

        ++cameras;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Search for camera "
                              << make << "-"
                              << model << " ==> false";

    return nullptr;
}

LensFunIface::LensPtr LensFunIface::findLens(const QString& model) const
{
    const lfLens* const* lenses = d->lfDb->GetLenses();

    while (lenses && *lenses)
    {
        LensPtr lens = *lenses;

        if (QString::fromLatin1(lens->Model) == model)
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Search for lens " << model << " ==> true";
            return lens;
        }

        ++lenses;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Search for lens " << model << " ==> false";

    return nullptr;
}

LensFunIface::MetadataMatch LensFunIface::findFromMetadata(const DMetadata* const meta)
{
    MetadataMatch ret  = MetadataNoMatch;
    d->settings        = LensFunContainer();
    d->usedCamera      = nullptr;
    d->usedLens        = nullptr;
    d->lensDescription.clear();

    if (!meta || meta->isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No metadata available";

        return LensFunIface::MetadataUnavailable;
    }

    PhotoInfoContainer photoInfo = meta->getPhotographInformation();
    d->makeDescription           = photoInfo.make.trimmed();
    d->modelDescription          = photoInfo.model.trimmed();
    bool exactMatch              = true;

    if (d->makeDescription.isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No camera maker info available";
        exactMatch = false;
    }
    else
    {
        // NOTE: see bug #184156:
        // Some rules to wrap unknown camera device from Lensfun database,
        // which have equivalent in fact.

        if (d->makeDescription == QLatin1String("Canon"))
        {
            if (d->modelDescription == QLatin1String("Canon EOS Kiss Digital X"))
            {
                d->modelDescription = QLatin1String("Canon EOS 400D DIGITAL");
            }

            if (d->modelDescription == QLatin1String("G1 X"))
            {
                d->modelDescription = QLatin1String("G1X");
            }
        }

        if (d->makeDescription.contains(QLatin1String("olympus"), Qt::CaseInsensitive))
        {
            if (!findCamera(d->makeDescription, d->modelDescription))
            {
                QStringList olympusList;
                olympusList << QLatin1String("Olympus Imaging Corp.");
                olympusList << QLatin1String("Olympus Corporation");
                olympusList << QLatin1String("Olympus");

                while (!olympusList.isEmpty())
                {
                    if (findCamera(olympusList.first(), d->modelDescription))
                    {
                        d->makeDescription = olympusList.first();
                        break;
                    }

                    olympusList.removeFirst();
                }
            }
        }

        d->lensDescription = photoInfo.lens.trimmed();

        // ------------------------------------------------------------------------------------------------

        DevicePtr lfCamera = findCamera(d->makeDescription, d->modelDescription);

        if (lfCamera)
        {
            setUsedCamera(lfCamera);

            qCDebug(DIGIKAM_DIMG_LOG) << "Camera maker   : " << d->settings.cameraMake;
            qCDebug(DIGIKAM_DIMG_LOG) << "Camera model   : " << d->settings.cameraModel;

            // ------------------------------------------------------------------------------------------------
            // -- Performing lens description searches.

            LensList lensMatches;

            if (!d->lensDescription.isEmpty())
            {
                QString  lensCutted;
                LensList lensList;

                // STAGE 1, search in LensFun database as well.

                lensList    = d->findLenses(d->usedCamera, d->lensDescription);

                qCDebug(DIGIKAM_DIMG_LOG) << "* Check for lens by direct query ("
                                          << d->lensDescription << " : "
                                          << lensList.count() << ")";

                lensMatches.append(lensList);

                // STAGE 2, Adapt exiv2 strings to lensfun strings for Nikon.

                lensCutted  = d->lensDescription;

                if (lensCutted.contains(QLatin1String("Nikon")))
                {
                    lensCutted.remove(QLatin1String("Nikon "));
                    lensCutted.remove(QLatin1String("Zoom-"));
                    lensCutted.replace(QLatin1String("IF-ID"), QLatin1String("ED-IF"));
                    lensList = d->findLenses(d->usedCamera, lensCutted);

                    qCDebug(DIGIKAM_DIMG_LOG) << "* Check for Nikon lens ("
                                              << lensCutted << " : "
                                              << lensList.count() << ")";

                    lensMatches.append(lensList);
                }

                // TODO : Add here more specific lens maker rules.

                // LAST STAGE, Adapt exiv2 strings to lensfun strings.
                // Some lens description use something like that :
                // "10.0 - 20.0 mm". This must be adapted like this : "10-20mm"

                lensCutted  = d->lensDescription;
                lensCutted.replace(QRegularExpression(QLatin1String("\\.[0-9]")), QLatin1String("")); //krazy:exclude=doublequote_chars
                lensCutted.replace(QLatin1String(" - "), QLatin1String("-"));
                lensCutted.replace(QLatin1String(" mm"), QLatin1String("mn"));
                lensList    = d->findLenses(d->usedCamera, lensCutted);

                qCDebug(DIGIKAM_DIMG_LOG) << "* Check for no maker lens ("
                                          << lensCutted << " : "
                                          << lensList.count() << ")";

                lensMatches.append(lensList);

                // Remove all duplicate lenses in the list by using QSet.

                QSet<const lfLens*> set(lensMatches.begin(), lensMatches.end());
                lensMatches = QList<const lfLens*>(set.begin(), set.end());
            }

            if (lensMatches.isEmpty())
            {
                qCDebug(DIGIKAM_DIMG_LOG) << "Lens description string is empty or no match";

                const LensList lensList = d->findLenses(d->usedCamera, QString());

                if (lensList.count() == 1)
                {
                    // NOTE: see bug #407157

                    qCDebug(DIGIKAM_DIMG_LOG) << "For the camera " << d->settings.cameraModel
                                              << " there is exactly one lens in the database: "
                                              << lensList.first()->Model;

                    lensMatches.append(lensList);
                }
                else
                {
                    exactMatch = false;
                }
            }

            // Display the results.

            if      (lensMatches.isEmpty())
            {
                qCDebug(DIGIKAM_DIMG_LOG) << "lens matches   : NOT FOUND";
                exactMatch = false;
            }
            else if (lensMatches.count() == 1)
            {
                // Best case for an exact match is to have only one item returned by Lensfun searches.

                setUsedLens(lensMatches.first());
                qCDebug(DIGIKAM_DIMG_LOG) << "Lens found     : " << d->settings.lensModel;
                qCDebug(DIGIKAM_DIMG_LOG) << "Crop Factor    : " << d->settings.cropFactor;
            }
            else
            {
                qCDebug(DIGIKAM_DIMG_LOG) << "lens matches   : more than one...";
                const lfLens* similar = nullptr;
                double percent        = 0.0;

                for (const lfLens* const l : std::as_const(lensMatches))
                {
                    double result = d->checkSimilarity(d->lensDescription, QLatin1String(l->Model));

                    if (result > percent)
                    {
                        percent = result;
                        similar = l;
                    }
                }

                if (similar)
                {
                    qCDebug(DIGIKAM_DIMG_LOG) << "found similar match from" << lensMatches.count()
                                              << "possibilities:" << similar->Model
                                              << "similarity:" << percent;
                    setUsedLens(similar);
                }
                else
                {
                    exactMatch = false;
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Cannot find Lensfun camera device for ("
                                      << d->makeDescription << " - "
                                      << d->modelDescription << ")";
            exactMatch = false;
        }
    }

    // ------------------------------------------------------------------------------------------------
    // Performing Lens settings searches.

    QString temp = photoInfo.focalLength;

    if (temp.isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Focal Length   : NOT FOUND";
        exactMatch = false;
    }

    // NOTE: strip the " mm" at the end ...
    d->settings.focalLength = temp.mid(0, temp.length() - 3).toDouble();

    qCDebug(DIGIKAM_DIMG_LOG) << "Focal Length   : " << d->settings.focalLength;

    // ------------------------------------------------------------------------------------------------

    temp = photoInfo.aperture;

    if (temp.isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Aperture       : NOT FOUND";
        exactMatch = false;
    }

    d->settings.aperture = temp.mid(1).toDouble();
    qCDebug(DIGIKAM_DIMG_LOG) << "Aperture       : " << d->settings.aperture;

    // ------------------------------------------------------------------------------------------------
    // Try to get subject distance value.

    // From standard Exif.

    temp = meta->getExifTagString("Exif.Photo.SubjectDistance");

    if (temp.isEmpty())
    {
        // From standard XMP.

        temp = meta->getXmpTagString("Xmp.exif.SubjectDistance");
    }

    if (temp.isEmpty())
    {
        // From Canon Makernote.

        temp = meta->getExifTagString("Exif.CanonSi.SubjectDistance");
    }

    if (temp.isEmpty())
    {
        // From Nikon Makernote.

        temp = meta->getExifTagString("Exif.NikonLd2.FocusDistance");
    }

    if (temp.isEmpty())
    {
        // From Nikon Makernote.

        temp = meta->getExifTagString("Exif.NikonLd3.FocusDistance");
    }

    if (temp.isEmpty())
    {
        // From Olympus Makernote.

        temp = meta->getExifTagString("Exif.OlympusFi.FocusDistance");
    }

    // TODO: Add here others Makernotes tags.

    if (temp.isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Subject dist.  : NOT FOUND : Use default value.";
        temp = QLatin1String("1000");
    }

    temp                        = temp.remove(QLatin1String(" m"));
    bool ok;
    d->settings.subjectDistance = temp.toDouble(&ok);

    if (!ok)
    {
        d->settings.subjectDistance = -1.0;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Subject dist.  : " << d->settings.subjectDistance;

    // ------------------------------------------------------------------------------------------------

    ret = exactMatch ? MetadataExactMatch : MetadataPartialMatch;

    qCDebug(DIGIKAM_DIMG_LOG) << "Metadata match : " << d->metadataMatchDebugStr(ret);

    return ret;
}

bool LensFunIface::supportsDistortion() const
{
    if (!d->usedLens)
    {
        return false;
    }

    lfLensCalibDistortion res;

    return d->usedLens->InterpolateDistortion(

#if LENSFUN_TEST_VERSION(0,3,99)
                                              d->settings.cropFactor,
#endif
                                              d->settings.focalLength,

                                              res
                                             );
}

bool LensFunIface::supportsCCA() const
{
    if (!d->usedLens)
    {
        return false;
    }

    lfLensCalibTCA res;

    return d->usedLens->InterpolateTCA(

#if LENSFUN_TEST_VERSION(0,3,99)
                                       d->settings.cropFactor,
#endif

                                       d->settings.focalLength,
                                       res
                                      );
}

bool LensFunIface::supportsVig() const
{
    if (!d->usedLens)
    {
        return false;
    }

    lfLensCalibVignetting res;

    return d->usedLens->InterpolateVignetting(

#if LENSFUN_TEST_VERSION(0,3,99)
                                              d->settings.cropFactor,
#endif

                                              d->settings.focalLength,
                                              d->settings.aperture,
                                              d->settings.subjectDistance,
                                              res
                                             );
}

bool LensFunIface::supportsGeometry() const
{
    return supportsDistortion();
}

QString LensFunIface::lensFunVersion()
{
    return QString::fromLatin1("%1.%2.%3-%4")
              .arg(LF_VERSION_MAJOR)
              .arg(LF_VERSION_MINOR)
              .arg(LF_VERSION_MICRO)
              .arg(LF_VERSION_BUGFIX);
}

// Restore warnings with older Lensfun API

#if !LENSFUN_TEST_VERSION(0,3,99)

#   if defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

} // namespace Digikam
