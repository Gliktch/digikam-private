/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Levels image filter
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "levelsfilter.h"

// Local includes

#include "dimg.h"
#include "imagelevels.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

LevelsFilter::LevelsFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

LevelsFilter::LevelsFilter(DImg* const orgImage,
                           QObject* const parent,
                           const LevelsContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("LevelsFilter")),
      m_settings        (settings)
{
    initFilter();
}

LevelsFilter::LevelsFilter(const LevelsContainer& settings,
                           DImgThreadedFilter* const master,
                           const DImg& orgImage,
                           DImg& destImage,
                           int progressBegin,
                           int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, QLatin1String("LevelsFilter")),
      m_settings        (settings)
{
    initFilter();
    destImage = m_destImage;
}

LevelsFilter::~LevelsFilter()
{
    cancelFilter();
}

QString LevelsFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Levels Adjust Tool"));
}

void LevelsFilter::filterImage()
{
    ImageLevels levels(m_orgImage.sixteenBit());

    for (int i = 0 ; i < 5 ; ++i)
    {
        postProgress(i * 10);
        levels.setLevelLowInputValue(i,   m_settings.lInput[i]);
        levels.setLevelHighInputValue(i,  m_settings.hInput[i]);
        levels.setLevelLowOutputValue(i,  m_settings.lOutput[i]);
        levels.setLevelHighOutputValue(i, m_settings.hOutput[i]);
        levels.setLevelGammaValue(i,      m_settings.gamma[i]);
    }

    postProgress(50);

    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                       m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    postProgress(60);

    levels.levelsCalculateTransfers();
    postProgress(70);

    // Process all channels Levels

    levels.levelsLutSetup(AlphaChannel);
    postProgress(80);

    levels.levelsLutProcess(m_orgImage.bits(), m_destImage.bits(),
                            m_orgImage.width(), m_orgImage.height());
    postProgress(90);
}

FilterAction LevelsFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    for (int i = 0 ; i < 5 ; ++i)
    {
        action.addParameter(QString::fromLatin1("gamma[%1]").arg(i),   m_settings.gamma[i]);
        action.addParameter(QString::fromLatin1("hInput[%1]").arg(i),  m_settings.hInput[i]);
        action.addParameter(QString::fromLatin1("hOutput[%1]").arg(i), m_settings.hOutput[i]);
        action.addParameter(QString::fromLatin1("lInput[%1]").arg(i),  m_settings.lInput[i]);
        action.addParameter(QString::fromLatin1("lOutput[%1]").arg(i), m_settings.lOutput[i]);
    }

    return action;
}

void LevelsFilter::readParameters(const Digikam::FilterAction& action)
{
    for (int i = 0 ; i < 5 ; ++i)
    {
        m_settings.gamma[i]   = action.parameter(QString::fromLatin1("gamma[%1]").arg(i)).toDouble();
        m_settings.hInput[i]  = action.parameter(QString::fromLatin1("hInput[%1]").arg(i)).toInt();
        m_settings.hOutput[i] = action.parameter(QString::fromLatin1("hOutput[%1]").arg(i)).toInt();
        m_settings.lInput[i]  = action.parameter(QString::fromLatin1("lInput[%1]").arg(i)).toInt();
        m_settings.lOutput[i] = action.parameter(QString::fromLatin1("lOutput[%1]").arg(i)).toInt();
    }
}

} // namespace Digikam

#include "moc_levelsfilter.cpp"
