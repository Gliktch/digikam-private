/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-26
 * Description : A HEIF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Other HEIF loader implementations:
 *     https://github.com/KDE/krita/tree/master/plugins/impex/heif
 *     https://github.com/jakar/qt-heif-image-plugin
 *     https://github.com/ImageMagick/ImageMagick/blob/master/coders/heic.c
 *     https://github.com/GNOME/gimp/blob/master/plug-ins/common/file-heif.c
 *
 * Specification: https://nokiatech.github.io/heif/technical.html
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Libheif includes

#include <libheif/heif.h>

// Local includes

#include "dimg.h"
#include "dimgloader.h"

using namespace Digikam;

namespace Digikam
{

class DImgHEIFLoader : public DImgLoader
{

public:

    explicit DImgHEIFLoader(DImg* const image);
    ~DImgHEIFLoader()                                                      override = default;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

    /**
     * Determine libx265 encoder bits depth capability: 8=standard, 10, 12, or more.
     * Return -1 if encoder instance is not found.
     */
    static int x265MaxBitsDepth();

private:

    bool isHeifSuccess(const struct heif_error* const error);

    /// Read operations

    bool readHEICColorProfile(struct heif_image_handle* const image_handle);
    bool readHEICImageByID(struct heif_context* const heif_context,
                           heif_item_id image_id);

    bool readHEICImageByHandle(struct heif_image_handle* image_handle,
                               struct heif_image* heif_image, bool loadImageData);

    /// Save operations

    bool saveHEICColorProfile(struct heif_image* const image);

    bool saveHEICMetadata(struct heif_context* const heif_context,
                          struct heif_image_handle* const image_handle);

private:

    bool                m_sixteenBit = false;
    bool                m_hasAlpha   = false;

    DImgLoaderObserver* m_observer   = nullptr;
};

} // namespace Digikam
