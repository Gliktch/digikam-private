/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>, Gilles Caulier
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <cstdio>
#include <cstdlib>

// C ANSI includes

extern "C"
{
// to avoid warnings under win32
#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H

#include <jpeglib.h>
}

// Local includes

#include "dimg.h"
#include "dimgloader.h"

using namespace Digikam;

namespace DigikamJPEGDImgPlugin
{

class DImgJPEGLoader : public DImgLoader
{

public:

    explicit DImgJPEGLoader(DImg* const image);
    ~DImgJPEGLoader()                                                      override = default;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    // To manage Errors/Warnings handling provide by libjpeg

    static void dimg_jpeg_error_exit(j_common_ptr cinfo);
    static void dimg_jpeg_emit_message(j_common_ptr cinfo, int msg_level);
    static void dimg_jpeg_output_message(j_common_ptr cinfo);
};

} // namespace DigikamJPEGDImgPlugin
