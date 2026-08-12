/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-15-02
 * Description : auto exposure image filter.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autoexpofilter.h"

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

// clazy:exclude=duplInheritedMember

AutoExpoFilter::AutoExpoFilter(QObject* const parent)
    : WBFilter(parent)
{
    initFilter();
}

AutoExpoFilter::AutoExpoFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent)
    : WBFilter  (orgImage, parent),
      m_refImage(*refImage)
{
    initFilter();
}

AutoExpoFilter::~AutoExpoFilter()
{
}

// cppcheck-suppress duplInheritedMember
QString AutoExpoFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Auto Exposure"));
}

void AutoExpoFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Ref. image and Org. have different bits depth";
        return;
    }

    autoExposureAdjustement(&m_refImage, m_settings.black, m_settings.expositionMain);
    WBFilter::filterImage();
}

FilterAction AutoExpoFilter::filterAction()
{
    return DefaultFilterAction<AutoExpoFilter>();
}

void AutoExpoFilter::readParameters(const FilterAction& action)
{
    Q_UNUSED(action);
}

} // namespace Digikam

#include "moc_autoexpofilter.cpp"
