/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on smoothslidesaver by Carsten Weinhold
 * <carsten dot weinhold at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "kbeffect.h"

// Qt includes

#include <QRandomGenerator>

// Local includes

#include "presentationkb.h"
#include "presentationkb_p.h"

namespace DigikamGenericPresentationPlugin
{

// -------------------------------------------------------------------------

int KBEffect::m_numKBEffectRepeated = 0;

// -------------------------------------------------------------------------

KBEffect::KBEffect(PresentationKB* const parent, bool needFadeIn)
    : m_needFadeIn (needFadeIn),
      m_slideWidget(parent)
{
}

void KBEffect::setupNewImage(int img)
{
    m_slideWidget->setupNewImage(img);
}

void KBEffect::swapImages()
{
    m_slideWidget->swapImages();
}

KBImage* KBEffect::image(int img) const
{
    Q_ASSERT ((img >= 0) && (img < 2));

    return m_slideWidget->d->image[img];
}

KBEffect::Type KBEffect::chooseKBEffect(KBEffect::Type oldType)
{
    KBEffect::Type effType;

    do
    {
        effType = (QRandomGenerator::global()->bounded(2U) == 0) ? KBEffect::Fade : KBEffect::Blend;
    }
    while ((effType == oldType) && (m_numKBEffectRepeated >= 1));

    if (effType == oldType)
    {
        m_numKBEffectRepeated++;
    }
    else
    {
        m_numKBEffectRepeated = 0;
    }

    return effType;
}

// -------------------------------------------------------------------------

FadeKBEffect::FadeKBEffect(PresentationKB* const parent, bool needFadeIn)
    : KBEffect(parent, needFadeIn)
{
    m_img[0] = image(0);
}

bool FadeKBEffect::done()
{
    if (m_img[0]->m_pos >= 1.0)
    {
        setupNewImage(0);

        return true;
    }

    return false;
}

void FadeKBEffect::advanceTime(float step)
{
    m_img[0]->m_pos += step;

    if (m_img[0]->m_pos >= 1.0)
    {
        m_img[0]->m_pos = 1.0;
    }

    if      (m_needFadeIn && (m_img[0]->m_pos < 0.1))
    {
        m_img[0]->m_opacity = m_img[0]->m_pos * 10;
    }
    else if (m_img[0]->m_pos > 0.9)
    {
        m_img[0]->m_opacity = (1.0 - m_img[0]->m_pos) * 10;
    }
    else
    {
        m_img[0]->m_opacity = 1.0;
    }
}

// -------------------------------------------------------------------------

BlendKBEffect::BlendKBEffect(PresentationKB* const parent, bool needFadeIn)
    : KBEffect(parent, needFadeIn)
{
    m_img[0] = image(0);
}

bool BlendKBEffect::done()
{
    if (m_img[0]->m_pos >= 1.0)
    {
        m_img[0]->m_paint = false;
        swapImages();

        return true;
    }

    return false;
}

void BlendKBEffect::advanceTime(float step)
{
    m_img[0]->m_pos += step;

    if (m_img[0]->m_pos >= 1.0)
    {
        m_img[0]->m_pos = 1.0;
    }

    if (m_img[1])
    {
        m_img[1]->m_pos += step;
    }

    if      (m_needFadeIn && (m_img[0]->m_pos < 0.1))
    {
        m_img[0]->m_opacity = m_img[0]->m_pos * 10;
    }
    else if (m_img[0]->m_pos > 0.9)
    {
        m_img[0]->m_opacity = (1.0 - m_img[0]->m_pos) * 10;

        if (m_img[1] == nullptr)
        {
            setupNewImage(1);
            m_img[1] = image(1);
            m_img[1]->m_opacity = 1.0;
        }
    }
    else
    {
        m_img[0]->m_opacity = 1.0;
    }
}

} // namespace DigikamGenericPresentationPlugin
