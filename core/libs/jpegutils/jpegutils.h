/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-29
 * Description : helper methods for JPEG image format.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"
#include "dmetadata.h"
#include "metaengine_rotation.h"

namespace Digikam
{

namespace JPEGUtils
{

typedef MetaEngineRotation::TransformationAction TransformAction;

class DIGIKAM_EXPORT JpegRotator
{
public:

    /**
     * Create a JpegRotator reading from the given file.
     * Per default, it will replace the file, read the current orientation from the metadata,
     * and use the src file name as documentName.
     */
    explicit JpegRotator(const QString& file);

    /**
     * Destructor
     */
    ~JpegRotator();

    /**
     * Per default, the orientation is read from the metadata of the file.
     * You can override this value
     */
    void setCurrentOrientation(MetaEngine::ImageOrientation orientation);

    /**
     * Set the Exif document name of the destination file.
     * Default value is the source's file name
     */
    void setDocumentName(const QString& documentName);

    /**
     * Set the destination file. By default, the source file will be overwritten
     * by atomic operation if the operation had succeeded.
     */
    void setDestinationFile(const QString& dest);

    /**
     * Rotate the JPEG file's content according to the current orientation,
     * resetting the current orientation to normal.
     * The final result of loading the image does not change.
     */
    bool autoExifTransform();

    /**
     * Rotate the given image by the given TransformAction.
     * The current orientation will be taken into account
     */
    bool exifTransform(TransformAction action);

    /**
     * Rotate the given image by the given Matrix.
     * The matrix describes the final transformation,
     * it is not adjusted by current rotation.
     */
    bool exifTransform(const MetaEngineRotation& matrix);

protected:

    QString                      m_file;
    QString                      m_destFile;
    QString                      m_documentName;
    QSize                        m_originalSize;
    DMetadata*                   m_metadata     = nullptr;
    MetaEngine::ImageOrientation m_orientation  = MetaEngine::ORIENTATION_UNSPECIFIED;

protected:

    void updateMetadata(const QString& fileName, const MetaEngineRotation& matrix);
    bool performJpegTransform(TransformAction action, const QString& src, const QString& dest);

private:

    /// @note disabled
    JpegRotator(const JpegRotator&)            = delete;
    JpegRotator& operator=(const JpegRotator&) = delete;
};

DIGIKAM_EXPORT bool loadJPEGScaled(QImage& image, const QString& path, int maximumSize);
DIGIKAM_EXPORT bool jpegConvert(const QString& src,
                                const QString& dest,
                                const QString& documentName,
                                const QString& format=QLatin1String("PNG"));
DIGIKAM_EXPORT bool isJpegImage(const QString& file);
DIGIKAM_EXPORT int  getJpegQuality(const QString& file);

} // namespace JPEGUtils

} // namespace Digikam
