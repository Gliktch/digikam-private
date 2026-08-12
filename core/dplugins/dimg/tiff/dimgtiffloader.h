/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Specifications & references:
 * - TIFF 6.0  : partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
 * - TIFF/EP   : www.map.tu.chiba-u.ac.jp/IEC/100/TA2/recdoc/N4378.pdf
 * - TIFF/Tags : www.awaresystems.be/imaging/tiff/tifftags.html
 * - DNG       : www.adobe.com/products/dng/pdfs/dng_spec.pdf
 *
 * Others Linux Tiff Loader implementation using libtiff:
 * - websvn.kde.org/trunk/koffice/filters/krita/tiff/kis_tiff_converter.cc
 * - artis.inrialpes.fr/Software/TiffIO/
 * - cvs.graphicsmagick.org/cgi-bin/cvsweb.cgi/GraphicsMagick/coders/tiff.c
 * - freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/FreeImage/PluginTIFF.cpp
 * - freeimage.cvs.sourceforge.net/freeimage/FreeImage/Source/Metadata/XTIFF.cpp
 * - subversion.imagemagick.org/subversion/ImageMagick/trunk/coders/tiff.c
 *
 * Test images repository:
 * - www.remotesensing.org/libtiff/images.html
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C ANSI includes

extern "C"
{
#include <tiffio.h>
#include <tiff.h>
}

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "dimgloader.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamTIFFDImgPlugin
{

class DImgTIFFLoader : public DImgLoader
{
public:

    explicit DImgTIFFLoader(DImg* const image);
    ~DImgTIFFLoader()                                                      override = default;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    void tiffSetExifAsciiTag(TIFF* const tif, ttag_t tiffTag, const DMetadata& metaData, const char* const exifTagName);

    // cppcheck-suppress unusedPrivateFunction
    void tiffSetExifDataTag(TIFF* const tif, ttag_t tiffTag, const DMetadata& metaData, const char* const exifTagName);

    static void dimg_tiff_warning(const char* module, const char* format, va_list warnings); // clazy:exclude=function-args-by-ref
    static void dimg_tiff_error(const char* module, const char* format, va_list errors);     // clazy:exclude=function-args-by-ref

private:

    bool m_sixteenBit = false;
    bool m_hasAlpha   = false;
};

} // namespace DigikamTIFFDImgPlugin
