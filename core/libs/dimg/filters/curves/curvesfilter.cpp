/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "curvesfilter.h"

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

CurvesFilter::CurvesFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

CurvesFilter::CurvesFilter(DImg* const orgImage,
                           QObject* const parent,
                           const CurvesContainer& settings)
    : DImgThreadedFilter(orgImage,
                         parent,
                         QLatin1String("CurvesFilter")),
      m_settings(settings)
{
    initFilter();
}

CurvesFilter::CurvesFilter(const CurvesContainer& settings,
                           DImgThreadedFilter* const master,
                           const DImg& orgImage,
                           DImg& destImage,
                           int progressBegin,
                           int progressEnd)
    : DImgThreadedFilter(master,
                         orgImage,
                         destImage,
                         progressBegin,
                         progressEnd,
                         QLatin1String("CurvesFilter")),
      m_settings(settings)
{
    // cannot operate in-place, so allocate dest image

    initFilter();
    destImage = m_destImage;
}

CurvesFilter::~CurvesFilter()
{
    cancelFilter();
}

QString CurvesFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Adjust Curves"));
}

void CurvesFilter::filterImage()
{
    postProgress(0);

    ImageCurves curves(m_settings);

    if (m_orgImage.sixteenBit() != m_settings.sixteenBit)
    {
        ImageCurves depthCurve(m_orgImage.sixteenBit());
        depthCurve.fillFromOtherCurves(&curves);
        curves = depthCurve;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Image 16 bits: " << m_orgImage.sixteenBit();
    qCDebug(DIGIKAM_DIMG_LOG) << "Curve 16 bits: " << curves.isSixteenBits();

    postProgress(50);

    // Process all channels curves

    curves.curvesLutSetup(AlphaChannel);
    postProgress(75);

    curves.curvesLutProcess(m_orgImage.bits(), m_destImage.bits(), m_orgImage.width(), m_orgImage.height());
    postProgress(100);
}

FilterAction CurvesFilter::filterAction()
{
    DefaultFilterAction<CurvesFilter> action(m_settings.isStoredLosslessly());
    m_settings.writeToFilterAction(action);

    return action;
}

void CurvesFilter::readParameters(const FilterAction& action)
{
    m_settings = CurvesContainer::fromFilterAction(action);
}

} // namespace Digikam

#include "moc_curvesfilter.cpp"
