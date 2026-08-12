/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Enhance image with local contrasts (as human eye does).
 *               LDR ToneMapper zynaddsubfx.sourceforge.net/other/tonemapping
 *
 * SPDX-FileCopyrightText: 2009      by Julien Pontabry <julien dot pontabry at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "localcontrastcontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    class Q_DECL_HIDDEN Args
    {
    public:

        Args() = default;

    public:

        uint   start            = 0;
        uint   stop             = 0;
        float  a                = 0.0F;
        float* data             = nullptr;
        int    sizex            = 0;
        int    sizey            = 0;
        float  blur             = 0.0F;
        float  denormal_remove  = 0.0F;
    };

public:

    explicit LocalContrastFilter(QObject* const parent = nullptr);
    explicit LocalContrastFilter(DImg* const image, QObject* const parent = nullptr,
                                 const LocalContrastContainer& par = LocalContrastContainer());
    ~LocalContrastFilter()                                                    override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:LocalContrastFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 2;
    }

    static int              CurrentVersion()
    {
        return 2;
    }

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;

    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;

    void  processRgbImage(float* const img, int sizex, int sizey);
    void  process8bitRgbImage(unsigned char* const img, int sizex, int sizey);
    void  process16bitRgbImage(unsigned short* const img, int sizex, int sizey);

    float func(float x1, float x2);

    void  inplaceBlur(float* const data, int sizex, int sizey, float blur);
    void  stretchContrast(float* const data, int datasize);

    inline void rgb2hsv(const float& r, const float& g, const float& b, float& h, float& s, float& v);
    inline void hsv2rgb(const float& h, const float& s, const float& v, float& r, float& g, float& b);

    void blurMultithreaded(uint start, uint stop, float* const img, float* const blurimage);
    void saturationMultithreaded(uint start, uint stop, float* const img, float* const srcimg);

    void inplaceBlurYMultithreaded(const Args& prm);
    void inplaceBlurXMultithreaded(const Args& prm);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
