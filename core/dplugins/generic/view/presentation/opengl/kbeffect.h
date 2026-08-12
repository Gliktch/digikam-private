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

#pragma once

// Qt includes

#include <QObject>

namespace DigikamGenericPresentationPlugin
{

class KBImage;
class PresentationKB;

class KBEffect
{

public:

    typedef enum
    {
        Fade,
        Blend
    } Type;

public:

    explicit KBEffect(PresentationKB* const parent, bool m_needFadeIn = true);
    virtual ~KBEffect()                  = default;

    virtual bool fadeIn()       const
    {
        return m_needFadeIn;
    };

    virtual void advanceTime(float step) = 0;
    virtual Type type()                  = 0;
    virtual bool done()                  = 0;

    static Type chooseKBEffect(Type oldType);

protected:

    void     setupNewImage(int img);
    void     swapImages();
    KBImage* image(int img)     const;

protected:

    static int      m_numKBEffectRepeated;
    bool            m_needFadeIn  = false;
    KBImage*        m_img[2]      = { nullptr };

private:

    PresentationKB* m_slideWidget = nullptr;

private:

    Q_DISABLE_COPY(KBEffect)
};

// -------------------------------------------------------------------------

class FadeKBEffect: public KBEffect
{

public:

    explicit FadeKBEffect(PresentationKB* const parent, bool m_needFadeIn = true);
    ~FadeKBEffect()              override = default;

    Type type()                  override
    {
        return Fade;
    };

    void advanceTime(float step) override;
    bool done()                  override;

private:

    Q_DISABLE_COPY(FadeKBEffect)
};

// -------------------------------------------------------------------------

class BlendKBEffect: public KBEffect
{

public:

    explicit BlendKBEffect(PresentationKB* const parent, bool m_needFadeIn = true);
    ~BlendKBEffect()             override = default;

    Type type()                  override
    {
        return Blend;
    };

    void advanceTime(float step) override;
    bool done()                  override;

private:

    Q_DISABLE_COPY(BlendKBEffect)
};

} // namespace DigikamGenericPresentationPlugin
