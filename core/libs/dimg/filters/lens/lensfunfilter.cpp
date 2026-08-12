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

#include "lensfunfilter.h"

// Qt includes

#include <QByteArray>
#include <QCheckBox>
#include <QString>
#include <QtConcurrentRun>
#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "lensfuniface.h"
#include "dmetadata.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "actionthreadbase.h"

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

class Q_DECL_HIDDEN LensFunFilter::Private
{
public:

    Private() = default;

    DImg          tempImage;

    LensFunIface* iface    = nullptr;

    lfModifier*   modifier = nullptr;

    int           loop     = 0;
};

LensFunFilter::LensFunFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    d->iface = new LensFunIface;
    initFilter();
}

LensFunFilter::LensFunFilter(DImg* const orgImage,
                             QObject* const parent,
                             const LensFunContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("LensCorrection")),
      d                 (new Private)
{
    d->iface = new LensFunIface;
    d->iface->setSettings(settings);
    initFilter();
}

LensFunFilter::~LensFunFilter()
{
    cancelFilter();

    if (d->modifier)
    {

#if LENSFUN_TEST_VERSION(0,3,99)

        delete d->modifier;

#else

        d->modifier->Destroy();

#endif

    }

    delete d->iface;
    delete d;
}

QString LensFunFilter::DisplayableName()
{
    static const struct FilterName
    {

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

        const KLazyLocalizedString    title;

#else

        const char*                   context;
        const char*                   title;

#endif

    }
    FILTER_NAME =
    {
        I18NC_NOOP("@title", "Lens Auto-Correction Tool")
    };

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)

    return FILTER_NAME.title.toString();

#else

    return i18nc(FILTER_NAME.context, FILTER_NAME.title);

#endif

}

void LensFunFilter::filterCCAMultithreaded(uint start, uint stop)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    QScopedArrayPointer<float> pos(new float[m_orgImage.width() * 2 * 3] { 0.0F });

    for (unsigned int y = start ; runningFlag() && (y < stop) ; ++y)
    {
        if (d->modifier->ApplySubpixelDistortion(0.0, y, m_orgImage.width(), 1, pos.data()))
        {
            float* src = pos.data();

            for (unsigned x = 0 ; runningFlag() && (x < m_destImage.width()) ; ++x)
            {
                DColor destPixel(0, 0, 0, 0xFFFF, m_destImage.sixteenBit());

                destPixel.setRed(m_orgImage.getSubPixelColorFast(src[0],   src[1]).red());
                destPixel.setGreen(m_orgImage.getSubPixelColorFast(src[2], src[3]).green());
                destPixel.setBlue(m_orgImage.getSubPixelColorFast(src[4],  src[5]).blue());

                m_destImage.setPixelColor(x, y, destPixel);
                src += 2 * 3;
            }
        }
    }
}

void LensFunFilter::filterVIGMultithreaded(uint start, uint stop)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    uchar* data = m_destImage.bits();
    data       += m_destImage.width() * m_destImage.bytesDepth() * start;

    for (unsigned int y = start ; runningFlag() && (y < stop) ; ++y)
    {
        if (d->modifier->ApplyColorModification(data,
                                                0.0,
                                                y,
                                                m_destImage.width(),
                                                1,
                                                LF_CR_4(RED, GREEN, BLUE, UNKNOWN),
                                                0))
        {
            data += m_destImage.width() * m_destImage.bytesDepth();
        }
    }
}

void LensFunFilter::filterDSTMultithreaded(uint start, uint stop)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    QScopedArrayPointer<float> pos(new float[m_orgImage.width() * 2 * 3] { 0.0F });

    for (unsigned int y = start ; runningFlag() && (y < stop) ; ++y)
    {
        if (d->modifier->ApplyGeometryDistortion(0.0, y, d->tempImage.width(), 1, pos.data()))
        {
            float* src = pos.data();

            for (unsigned int x = 0 ; runningFlag() && (x < d->tempImage.width()) ; ++x, ++d->loop)
            {
                d->tempImage.setPixelColor(x, y, m_destImage.getSubPixelColor(src[0], src[1]));
                src += 2;
            }
        }
    }
}

void LensFunFilter::filterImage()
{
    m_destImage.bitBltImage(&m_orgImage, 0, 0);

    if (!d->iface)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: LensFun Interface is null.";
        return;
    }

    if (!d->iface->usedLens())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: LensFun Interface Lens device is null.";
        return;
    }

    // Init lensfun lib, we are working on the full image.

    lfPixelFormat colorDepth = (m_orgImage.bytesDepth() == 4) ? LF_PF_U8 : LF_PF_U16;
    double focalLength       = d->iface->settings().focalLength;
    double cropFactor        = d->iface->settings().cropFactor;

    if (d->iface->settings().cameraModel == QLatin1String("DSC-RX100M6"))
    {
        focalLength = focalLength * cropFactor;
    }

#if LENSFUN_TEST_VERSION(0,3,99)

    d->modifier              = new lfModifier(
                                              d->iface->usedLens(),
                                              focalLength,
                                              cropFactor,
                                              m_orgImage.width(),
                                              m_orgImage.height(),
                                              colorDepth,
                                              false                    // no inverse
                                             );

#else

    // Lensfun Modifier flags to process

    int modifyFlags = 0;

    if (d->iface->settings().filterDST)
    {
        modifyFlags |= LF_MODIFY_DISTORTION;
    }

    if (d->iface->settings().filterGEO)
    {
        modifyFlags |= LF_MODIFY_GEOMETRY;
    }

    if (d->iface->settings().filterCCA)
    {
        modifyFlags |= LF_MODIFY_TCA;
    }

    if (d->iface->settings().filterVIG)
    {
        modifyFlags |= LF_MODIFY_VIGNETTING;
    }

    d->modifier              = lfModifier::Create(d->iface->usedLens(),
                                                  cropFactor,
                                                  m_orgImage.width(),
                                                  m_orgImage.height());

    int modflags             = d->modifier->Initialize(
                                                       d->iface->usedLens(),
                                                       colorDepth,
                                                       focalLength,
                                                       d->iface->settings().aperture,
                                                       d->iface->settings().subjectDistance,
                                                       1.0,             /* no scaling */
                                                       LF_RECTILINEAR,
                                                       modifyFlags,
                                                       0                /* no inverse */
                                                      );

#endif

    if (!d->modifier)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: cannot initialize LensFun Modifier.";
        return;

    }

#if LENSFUN_TEST_VERSION(0,3,99)

    // Lensfun Modifier flags to process

    int modflags = 0;
    modflags    |= d->modifier->EnableScaling(1.0);             // no scaling

    if (d->iface->settings().filterDST)
    {
        modflags |= d->modifier->EnableDistortionCorrection();
    }

    if (d->iface->settings().filterGEO)
    {
        modflags |= d->modifier->EnableProjectionTransform(LF_RECTILINEAR);
    }

    if (d->iface->settings().filterCCA)
    {
        modflags |= d->modifier->EnableTCACorrection();
    }

    if (d->iface->settings().filterVIG)
    {
        modflags |= d->modifier->EnableVignettingCorrection(
                                                            d->iface->settings().aperture,
                                                            d->iface->settings().subjectDistance
                                                           );
    }

#endif

    // Calc necessary steps for progress bar

    int steps = ((d->iface->settings().filterCCA)                                   ? 1 : 0) +
                ((d->iface->settings().filterVIG)                                   ? 1 : 0) +
                ((d->iface->settings().filterDST || d->iface->settings().filterGEO) ? 1 : 0);

    qCDebug(DIGIKAM_DIMG_LOG) << "LensFun Modifier Flags: " << modflags << "  Steps:" << steps;

    if (steps < 1)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No LensFun Modifier steps. There is nothing to process...";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Image size to process: ("
                              << m_orgImage.width()  << ", "
                              << m_orgImage.height() << ")";

    QList<int> vals = multithreadedSteps(m_destImage.height());

    // Stage 1: Chromatic Aberration Corrections

    if (d->iface->settings().filterCCA)
    {
        m_orgImage.prepareSubPixelAccess(); // init lanczos kernel

        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < (vals.count() - 1)) ; ++j)
        {
            tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                           &LensFunFilter::filterCCAMultithreaded, this,

#else

                                           this, &LensFunFilter::filterCCAMultithreaded,

#endif

                                           vals[j    ],
                                           vals[j + 1])
            );
        }

        for (QFuture<void> t : std::as_const(tasks))
        {
            t.waitForFinished();
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Chromatic Aberration Corrections applied.";
    }

    postProgress(30);

    // Stage 2: Color Corrections: Vignetting and Color Contribution Index

    if (d->iface->settings().filterVIG)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < (vals.count() - 1)) ; ++j)
        {
            tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                           &LensFunFilter::filterVIGMultithreaded, this,

#else

                                           this, &LensFunFilter::filterVIGMultithreaded,

#endif

                                           vals[j    ],
                                           vals[j + 1])
            );
        }

        for (QFuture<void> t : std::as_const(tasks))
        {
            t.waitForFinished();
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Vignetting and Color Corrections applied.";
    }

    postProgress(60);

    // Stage 3: Distortion and Geometry Corrections

    if (d->iface->settings().filterDST || d->iface->settings().filterGEO)
    {
        d->loop = 0;

        // we need a deep copy first

        d->tempImage = DImg(m_destImage.width(),
                            m_destImage.height(),
                            m_destImage.sixteenBit(),
                            m_destImage.hasAlpha());

        m_destImage.prepareSubPixelAccess(); // init lanczos kernel

        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < (vals.count() - 1)) ; ++j)
        {
            tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                           &LensFunFilter::filterDSTMultithreaded, this,

#else

                                           this, &LensFunFilter::filterDSTMultithreaded,

#endif

                                           vals[j    ],
                                           vals[j + 1])
            );
        }

        for (QFuture<void> t : std::as_const(tasks))
        {
            t.waitForFinished();
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Distortion and Geometry Corrections applied.";

        if (d->loop)
        {
            m_destImage = d->tempImage;
        }
    }

    postProgress(90);
}

bool LensFunFilter::registerSettingsToXmp(MetaEngineData& data) const
{
    // Register in digiKam Xmp namespace all information about Lens corrections.

    QString str;
    LensFunContainer prm = d->iface->settings();

    str.append(i18nc("@info; XMP settings", "Camera: %1-%2",        prm.cameraMake, prm.cameraModel));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "Lens: %1",             prm.lensModel));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "Subject Distance: <numid>%1</numid>", prm.subjectDistance));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "Aperture: <numid>%1</numid>",         prm.aperture));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "Focal Length: <numid>%1</numid>",     prm.focalLength));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "Crop Factor: <numid>%1</numid>",      prm.cropFactor));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "CCA Correction: %1",   prm.filterCCA  && d->iface->supportsCCA()       ? i18nc("@info: correction status", "enabled")
                                                                                                                    : i18nc("@info: correction status", "disabled")));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "VIG Correction: %1",   prm.filterVIG  && d->iface->supportsVig()       ? i18nc("@info: correction status", "enabled")
                                                                                                                    : i18nc("@info: correction status", "disabled")));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "DST Correction: %1",   prm.filterDST && d->iface->supportsDistortion() ? i18nc("@info: correction status", "enabled")
                                                                                                                    : i18nc("@info: correction status", "disabled")));
    str.append(QLatin1Char('\n'));
    str.append(i18nc("@info; XMP settings", "GEO Correction: %1",   prm.filterGEO && d->iface->supportsGeometry()   ? i18nc("@info: correction status", "enabled")
                                                                                                                    : i18nc("@info: correction status", "disabled")));

    QScopedPointer<DMetadata> meta(new DMetadata(data));
    bool ret = meta->setXmpTagString("Xmp.digiKam.LensCorrectionSettings",
                                    str.replace(QLatin1Char('\n'), QLatin1String(" ; ")));
    data     = meta->data();

    return ret;
}

FilterAction LensFunFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    LensFunContainer prm = d->iface->settings();
    action.addParameter(QLatin1String("ccaCorrection"),   prm.filterCCA);
    action.addParameter(QLatin1String("vigCorrection"),   prm.filterVIG);
    action.addParameter(QLatin1String("dstCorrection"),   prm.filterDST);
    action.addParameter(QLatin1String("geoCorrection"),   prm.filterGEO);
    action.addParameter(QLatin1String("cropFactor"),      prm.cropFactor);
    action.addParameter(QLatin1String("focalLength"),     prm.focalLength);
    action.addParameter(QLatin1String("aperture"),        prm.aperture);
    action.addParameter(QLatin1String("subjectDistance"), prm.subjectDistance);
    action.addParameter(QLatin1String("cameraMake"),      prm.cameraMake);
    action.addParameter(QLatin1String("cameraModel"),     prm.cameraModel);
    action.addParameter(QLatin1String("lensModel"),       prm.lensModel);

    return action;
}

void LensFunFilter::readParameters(const Digikam::FilterAction& action)
{
    LensFunContainer prm = d->iface->settings();
    prm.filterCCA        = action.parameter(QLatin1String("ccaCorrection")).toBool();
    prm.filterVIG        = action.parameter(QLatin1String("vigCorrection")).toBool();
    prm.filterDST        = action.parameter(QLatin1String("dstCorrection")).toBool();
    prm.filterGEO        = action.parameter(QLatin1String("geoCorrection")).toBool();
    prm.cropFactor       = action.parameter(QLatin1String("cropFactor")).toDouble();
    prm.focalLength      = action.parameter(QLatin1String("focalLength")).toDouble();
    prm.aperture         = action.parameter(QLatin1String("aperture")).toDouble();
    prm.subjectDistance  = action.parameter(QLatin1String("subjectDistance")).toDouble();
    prm.cameraMake       = action.parameter(QLatin1String("cameraMake")).toString();
    prm.cameraModel      = action.parameter(QLatin1String("cameraModel")).toString();
    prm.lensModel        = action.parameter(QLatin1String("lensModel")).toString();
    d->iface->setSettings(prm);
}

} // namespace Digikam

// Restore warnings with older Lensfun API

#if !LENSFUN_TEST_VERSION(0,3,99)

#   if defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

#include "moc_lensfunfilter.cpp"
