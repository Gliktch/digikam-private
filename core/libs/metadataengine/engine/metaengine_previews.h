/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Embedded preview loading.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QSize>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetaEnginePreviews
{
public:

    /**
     * Open the given file and scan for embedded preview images
     */
    explicit MetaEnginePreviews(const QString& filePath);

    /**
     * Open the given image data and scan the image for embedded preview images.
     */
    explicit MetaEnginePreviews(const QByteArray& imgData);
    ~MetaEnginePreviews();

    /// Returns the pixel size of the original image, as read from the file (not the metadata).
    QSize originalSize()        const;

    /// Returns the mimeType of the original image, detected from the file's content.
    QString originalMimeType()  const;

    /// Returns if there are any preview images available
    bool       isEmpty();

    /// Returns how many embedded previews are available
    int        count()          const;
    int        size()           const;

    /**
     * For each contained preview image, return the size
     * of the image data in bytes, width and height of the preview,
     * the mimeType and the file extension.
     * Ensure that index < count().
     * Previews are sorted by width*height, largest first.
     */
    int        dataSize(int index = 0);
    int        width(int index = 0);
    int        height(int index = 0);
    QString    mimeType(int index = 0);
    QString    fileExtension(int index = 0);

    /**
     * Retrieve the image data for the specified embedded preview image
     */
    QByteArray data(int index = 0);

    /**
     * Loads the data of the specified preview and creates a QImage
     * from this data. Returns a null QImage if the loading failed.
     */
    QImage image(int index = 0);

private:

    /// @note disabled
    MetaEnginePreviews(const MetaEnginePreviews&)            = delete;
    MetaEnginePreviews& operator=(const MetaEnginePreviews&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
