/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : ICC Transform threaded image filter.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "icctransformfilter.h"

// C++ includes

#include <cmath>

// Local includes

#include "dimg.h"
#include "iccsettings.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

IccTransformFilter::IccTransformFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

IccTransformFilter::IccTransformFilter(DImg* const orgImage, QObject* const parent, const IccTransform& transform)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("ICC Transform")),
      m_transform       (transform)
{
    initFilter();
}

IccTransformFilter::~IccTransformFilter()
{
    cancelFilter();
}

QString IccTransformFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Color Profile Conversion"));
}

void IccTransformFilter::filterImage()
{
    m_destImage = m_orgImage;
    m_transform.apply(m_destImage, this);
    m_destImage.setIccProfile(m_transform.outputProfile());
}

void IccTransformFilter::progressInfo(float progress)
{
    postProgress(lround(progress * 100));
}

FilterAction IccTransformFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("renderingIntent"),          m_transform.intent());
    action.addParameter(QLatin1String("blackPointCompensation"),   m_transform.isUsingBlackPointCompensation());
    action.addParameter(QLatin1String("inputProfileDescription"),  m_transform.effectiveInputProfile().description());
    action.addParameter(QLatin1String("outputProfileDescription"), m_transform.outputProfile().description());

    return action;
}

void IccTransformFilter::readParameters(const Digikam::FilterAction& action)
{
    m_transform = IccTransform();

    m_transform.setIntent((IccTransform::RenderingIntent)action.parameter(QLatin1String("renderingIntent")).toInt());
    m_transform.setUseBlackPointCompensation(action.parameter(QLatin1String("blackPointCompensation")).toBool());

    QList<IccProfile> profiles;
    profiles    = IccSettings::instance()->profilesForDescription(action.parameter(QLatin1String("inputProfileDescription")).toString());

    if (!profiles.isEmpty())
    {
        m_transform.setInputProfile(profiles.first());
    }

    profiles = IccSettings::instance()->profilesForDescription(action.parameter(QLatin1String("outputProfileDescription")).toString());

    if (!profiles.isEmpty())
    {
        m_transform.setOutputProfile(profiles.first());
    }
}

bool IccTransformFilter::parametersSuccessfullyRead() const
{
    return (!m_transform.inputProfile().isNull() && !m_transform.outputProfile().isNull());
}

QString IccTransformFilter::readParametersError(const FilterAction& actionThatFailed) const
{
    if (m_transform.inputProfile().isNull())
    {
        return i18n("Input color profile \"%1\" not available",
                    actionThatFailed.parameter(QLatin1String("inputProfileDescription")).toString());
    }

    if (m_transform.outputProfile().isNull())
    {
        return i18n("Output color profile \"%1\" not available",
                    actionThatFailed.parameter(QLatin1String("outputProfileDescription")).toString());
    }

    return QString();
}

} // namespace Digikam

#include "moc_icctransformfilter.cpp"
