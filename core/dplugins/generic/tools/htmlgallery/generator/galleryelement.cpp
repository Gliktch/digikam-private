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

// Local includes

#include "galleryelement.h"
#include "galleryxmlutils.h"
#include "metaengine.h"

namespace DigikamGenericHtmlGalleryPlugin
{

GalleryElement::GalleryElement(const DInfoInterface::DInfoMap& info)
{
    DItemInfo item(info);
    m_title       = item.name();
    m_imageTitle  = item.title();
    m_description = item.comment();
    m_orientation = (MetaEngine::ImageOrientation)(item.orientation());
    m_time        = item.dateTime();
}

void GalleryElement::appendToXML(XMLWriter& xmlWriter, bool copyOriginalImage) const
{
    if (!m_valid)
    {
        return;
    }

    XMLElement imageX(xmlWriter, QLatin1String("image"));
    xmlWriter.writeElement("title",       m_title);
    xmlWriter.writeElement("imagetitle",  m_imageTitle);
    xmlWriter.writeElement("description", m_description);
    xmlWriter.writeElement("date",        m_time.toString(QLatin1String("yyyy-MM-ddThh:mm:ss")));
    appendImageElementToXML(xmlWriter, QLatin1String("full"),      m_fullFileName, m_fullSize);
    appendImageElementToXML(xmlWriter, QLatin1String("thumbnail"), m_thumbnailFileName, m_thumbnailSize);

    if (copyOriginalImage)
    {
        appendImageElementToXML(xmlWriter,
                                QLatin1String("original"),
                                m_originalFileName, m_originalSize);
    }

    // Exif
    // TODO put all exif tags in a sub level

    XMLElement imageExif(xmlWriter, QLatin1String("exif"));
    xmlWriter.writeElement("exifimagemake",              m_exifImageMake);
    xmlWriter.writeElement("exifimagemodel",             m_exifItemModel);
    xmlWriter.writeElement("exifimageorientation",       m_exifImageOrientation);
    xmlWriter.writeElement("exifimagexresolution",       m_exifImageXResolution);
    xmlWriter.writeElement("exifimageyresolution",       m_exifImageYResolution);
    xmlWriter.writeElement("exifimageresolutionunit",    m_exifImageResolutionUnit);
    xmlWriter.writeElement("exifimagedatetime",          m_exifImageDateTime);
    xmlWriter.writeElement("exifimageycbcrpositioning",  m_exifImageYCbCrPositioning);
    xmlWriter.writeElement("exifphotoexposuretime",      m_exifPhotoExposureTime);
    xmlWriter.writeElement("exifphotofnumber",           m_exifPhotoFNumber);
    xmlWriter.writeElement("exifphotoexposureprogram",   m_exifPhotoExposureProgram);
    xmlWriter.writeElement("exifphotoisospeedratings",   m_exifPhotoISOSpeedRatings);
    xmlWriter.writeElement("exifphotoshutterspeedvalue", m_exifPhotoShutterSpeedValue);
    xmlWriter.writeElement("exifphotoaperturevalue",     m_exifPhotoApertureValue);
    xmlWriter.writeElement("exifphotofocallength",       m_exifPhotoFocalLength);

    // GPS

    xmlWriter.writeElement("exifgpslatitude",            m_exifGPSLatitude);
    xmlWriter.writeElement("exifgpslongitude",           m_exifGPSLongitude);
    xmlWriter.writeElement("exifgpsaltitude",            m_exifGPSAltitude);
}

void GalleryElement::appendImageElementToXML(XMLWriter& xmlWriter,
                                             const QString& elementName,
                                             const QString& fileName,
                                             const QSize& size) const
{
    XMLAttributeList attrList;
    attrList.append(QLatin1String("fileName"), fileName);
    attrList.append(QLatin1String("width"),    size.width());
    attrList.append(QLatin1String("height"),   size.height());
    XMLElement elem(xmlWriter, elementName, &attrList);
}

} // namespace DigikamGenericHtmlGalleryPlugin
