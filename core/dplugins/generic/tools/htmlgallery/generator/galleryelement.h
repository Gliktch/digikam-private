/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSize>
#include <QString>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

class XMLWriter;

/**
 * This class stores all the necessary information to produce an XML
 * description of a gallery element.
 */
class GalleryElement
{
public:

    explicit GalleryElement(const DInfoInterface::DInfoMap& info);
    GalleryElement()  = default;
    ~GalleryElement() = default;

    void appendToXML(XMLWriter& xmlWriter, bool copyOriginalImage) const;

    void appendImageElementToXML(XMLWriter& xmlWriter, const QString& elementName,
                                 const QString& fileName, const QSize& size) const;

public:

    bool                         m_valid        = false;
    QString                      m_title;
    QString                      m_imageTitle;
    QString                      m_description;
    DMetadata::ImageOrientation  m_orientation  = MetaEngine::ORIENTATION_UNSPECIFIED;
    QDateTime                    m_time;

    QString                      m_path;

    QString                      m_thumbnailFileName;
    QSize                        m_thumbnailSize;
    QString                      m_fullFileName;
    QSize                        m_fullSize;
    QString                      m_originalFileName;
    QSize                        m_originalSize;

    // Exif Metadata

    QString                      m_exifImageMake;
    QString                      m_exifItemModel;
    QString                      m_exifImageOrientation;
    QString                      m_exifImageXResolution;
    QString                      m_exifImageYResolution;
    QString                      m_exifImageResolutionUnit;
    QString                      m_exifImageDateTime;
    QString                      m_exifImageYCbCrPositioning;
    QString                      m_exifPhotoExposureTime;
    QString                      m_exifPhotoFNumber;
    QString                      m_exifPhotoExposureProgram;
    QString                      m_exifPhotoISOSpeedRatings;
    QString                      m_exifPhotoShutterSpeedValue;
    QString                      m_exifPhotoApertureValue;
    QString                      m_exifPhotoFocalLength;

    // GPS Metadata

    QString                      m_exifGPSLatitude;
    QString                      m_exifGPSLongitude;
    QString                      m_exifGPSAltitude;
};

} // namespace DigikamGenericHtmlGalleryPlugin
