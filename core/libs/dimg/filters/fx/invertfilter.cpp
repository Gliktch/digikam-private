/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : An invert image threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "invertfilter.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

InvertFilter::InvertFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

InvertFilter::InvertFilter(DImg* const orgImage, QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("InvertFilter"))
{
    initFilter();
}

InvertFilter::InvertFilter(DImgThreadedFilter* const parentFilter,
                           const DImg& orgImage, DImg& destImage,
                           int progressBegin, int progressEnd)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": InvertFilter"))
{
    initFilter();
    destImage = m_destImage;
}

InvertFilter::~InvertFilter()
{
    cancelFilter();
}

QString InvertFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Invert Effect"));
}

/**
 * Performs image colors inversion. This tool is used for negate image
 * resulting of a positive film scanned.
 */
void InvertFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    if (!m_destImage.sixteenBit())        // 8 bits image.
    {
        uchar* ptr = m_destImage.bits();

        for (uint i = 0 ; i < m_destImage.numPixels() ; ++i)
        {
            ptr[0] = 255 - ptr[0];
            ptr[1] = 255 - ptr[1];
            ptr[2] = 255 - ptr[2];

            // XXX: don't invert alpha channel! ptr[3] = 255 - ptr[3];

            ptr   += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short* ptr = reinterpret_cast<unsigned short*>(m_destImage.bits());

        for (uint i = 0 ; i < m_destImage.numPixels() ; ++i)
        {
            ptr[0] = 65535 - ptr[0];
            ptr[1] = 65535 - ptr[1];
            ptr[2] = 65535 - ptr[2];

            // XXX: don't invert alpha channel! ptr[3] = 65535 - ptr[3];

            ptr   += 4;
        }
    }
}

FilterAction InvertFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    return action;
}

void InvertFilter::readParameters(const FilterAction& /*action*/)
{
}

} // namespace Digikam

#include "moc_invertfilter.cpp"
