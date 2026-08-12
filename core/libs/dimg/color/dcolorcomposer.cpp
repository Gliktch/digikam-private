/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-03-02
 * Description : DColor methods for composing
 *               Integer arithmetic inspired by DirectFB,
 *               src/gfx/generic/generic.c and src/display/idirectfbsurface.c
 *
 * SPDX-FileCopyrightText: 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2000-2002 by convergence integrated media GmbH <curanz@convergence.de>
 * SPDX-FileCopyrightText: 2002-2005 by Denis Oliver Kropp <dok at directfb dot org>
 * SPDX-FileCopyrightText: 2002-2005 by Andreas Hundt <andi at fischlustig dot de>
 * SPDX-FileCopyrightText: 2002-2005 by Sven Neumann <neo at directfb dot org>
 * SPDX-FileCopyrightText: 2002-2005 by Ville Syrj <syrjala at sci dot fi>
 * SPDX-FileCopyrightText: 2002-2005 by Claudio Ciccani <klan at users dot sf dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dcolorcomposer.h"

namespace Digikam
{

class Q_DECL_HIDDEN DColorComposerPorterDuffNone : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffClear : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffSrc : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffSrcOver : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffDstOver : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffSrcIn : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffDstIn : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffSrcOut : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffDstOut : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffSrcAtop : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffDstAtop : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

class Q_DECL_HIDDEN DColorComposerPorterDuffXor : public DColorComposer
{
public:

    void compose(DColor& dest, DColor& src)                                          override;
    void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags) override;
};

/**
 * Porter-Duff None
 * component = (source * sa + destination * (1-sa))
 * Src blending function Src Alpha
 * Dst blending function Inv Src Alpha
 */
void DColorComposerPorterDuffNone::compose(DColor& dest, DColor& src)
{
    // preserve src alpha value for dest blending,
    // src.alpha() will be changed after blending src

    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendAlpha16(sa);
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(sa);
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffNone::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Clear
 * component = (source * 0 + destination * 0)
 * Src blending function Zero
 * Dst blending function Zero
 */
void DColorComposerPorterDuffClear::compose(DColor& dest, DColor& src)
{
    src.blendZero();
    dest.blendZero();
    dest.blendAdd(src);
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffClear::compose(DColor& dest, DColor& src, MultiplicationFlags)
{
    // skip pre- and demultiplication

    compose(dest, src);
}

/**
 * Porter-Duff Src
 * Normal Painter's algorithm
 * component = (source * 1 + destination * 0)
 * Src blending function One
 * Dst blending function Zero
 */
void DColorComposerPorterDuffSrc::compose(DColor& dest, DColor& src)
{
    // src: no-op

    dest.blendZero();
    dest.blendAdd(src);
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffSrc::compose(DColor& dest, DColor& src, MultiplicationFlags)
{
    // skip pre- and demultiplication

    compose(dest, src);
}

/**
 * Porter-Duff Src Over
 * component = (source * 1 + destination * (1-sa))
 * Src blending function One
 * Dst blending function Inv Src Alpha
 */
void DColorComposerPorterDuffSrcOver::compose(DColor& dest, DColor& src)
{
    if (dest.sixteenBit())
    {
        // src: no-op

        dest.blendInvAlpha16(src.alpha());
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        // src: no-op

        dest.blendInvAlpha8(src.alpha());
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffSrcOver::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Dst over
 * component = (source * (1.0-da) + destination * 1)
 * Src blending function Inv Dst Alpha
 * Dst blending function One
 */
void DColorComposerPorterDuffDstOver::compose(DColor& dest, DColor& src)
{
    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());

        // dest: no-op

        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());

        // dest: no-op

        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffDstOver::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Src In
 * component = (source * da + destination * 0)
 * Src blending function Dst Alpha
 * Dst blending function Zero
 */
void DColorComposerPorterDuffSrcIn::compose(DColor& dest, DColor& src)
{
    if (dest.sixteenBit())
    {
        src.blendAlpha16(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffSrcIn::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Dst In
 * component = (source * 0 + destination * sa)
 * Src blending function Zero
 * Dst blending function Src Alpha
 */
void DColorComposerPorterDuffDstIn::compose(DColor& dest, DColor& src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendZero();
        dest.blendAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendZero();
        dest.blendAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffDstIn::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Src Out
 * component = (source * (1-da) + destination * 0)
 * Src blending function Inv Dst Alpha
 * Dst blending function Zero
 */
void DColorComposerPorterDuffSrcOut::compose(DColor& dest, DColor& src)
{
    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendZero();
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffSrcOut::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Dst Out
 * component = (source * 0 + destination * (1-sa))
 * Src blending function Zero
 * Dst blending function Inv Src Alpha
 */
void DColorComposerPorterDuffDstOut::compose(DColor& dest, DColor& src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendZero();
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendZero();
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffDstOut::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Src Atop
 * component = (source * da + destination * (1-sa))
 * Src blending function Dst Alpha
 * Dst blending function Inv Src Alpha
 */
void DColorComposerPorterDuffSrcAtop::compose(DColor& dest, DColor& src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendAlpha16(dest.alpha());
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffSrcAtop::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Dst Atop
 * component = (source * (1-da) + destination * sa)
 * Src blending function Inv Dest Alpha
 * Dst blending function Src Alpha
 */
void DColorComposerPorterDuffDstAtop::compose(DColor& dest, DColor& src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffDstAtop::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

/**
 * Porter-Duff Xor
 * component = (source * (1-da) + destination * (1-sa))
 * Src blending function Inv Dst Alpha
 * Dst blending function Inv Src Alpha
 */
void DColorComposerPorterDuffXor::compose(DColor& dest, DColor& src)
{
    int sa = src.alpha();

    if (dest.sixteenBit())
    {
        src.blendInvAlpha16(dest.alpha());
        dest.blendInvAlpha16(sa);
        dest.blendAdd(src);
        dest.blendClamp16();
    }
    else
    {
        src.blendInvAlpha8(dest.alpha());
        dest.blendInvAlpha8(sa);
        dest.blendAdd(src);
        dest.blendClamp8();
    }
}

// cppcheck-suppress passedByValue
void DColorComposerPorterDuffXor::compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags)
{
    // Explicit implementation to please gcc 4.1

    DColorComposer::compose(dest, src, multiplicationFlags);
}

// ----------------------------------------------------------

// cppcheck-suppress passedByValue
void DColorComposer::compose(DColor& dest, DColor& src, DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (multiplicationFlags & PremultiplySrc)
    {
        src.premultiply();
    }

    if (multiplicationFlags & PremultiplyDst)
    {
        dest.premultiply();
    }

    compose(dest, src);

    if (multiplicationFlags & DemultiplyDst)
    {
        dest.demultiply();
    }
}

DColorComposer* DColorComposer::getComposer(DColorComposer::CompositingOperation rule)
{
    switch (rule)
    {
        case PorterDuffNone:
        {
            return new DColorComposerPorterDuffNone;
        }

        case PorterDuffClear:
        {
            return new DColorComposerPorterDuffClear;
        }

        case PorterDuffSrc:
        {
            return new DColorComposerPorterDuffSrc;
        }

        case PorterDuffSrcOver:
        {
            return new DColorComposerPorterDuffSrcOver;
        }

        case PorterDuffDstOver:
        {
            return new DColorComposerPorterDuffDstOver;
        }

        case PorterDuffSrcIn:
        {
            return new DColorComposerPorterDuffSrcIn;
        }

        case PorterDuffDstIn:
        {
            return new DColorComposerPorterDuffDstIn;
        }

        case PorterDuffSrcOut:
        {
            return new DColorComposerPorterDuffSrcOut;
        }

        case PorterDuffDstOut:
        {
            return new DColorComposerPorterDuffDstOut;
        }

        case PorterDuffSrcAtop:
        {
            return new DColorComposerPorterDuffDstOut;
        }

        case PorterDuffDstAtop:
        {
            return new DColorComposerPorterDuffDstOut;
        }

        case PorterDuffXor:
        {
            return new DColorComposerPorterDuffDstOut;
        }
    }

    return nullptr;
}

} // namespace Digikam
