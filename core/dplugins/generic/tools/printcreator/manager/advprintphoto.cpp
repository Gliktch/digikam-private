/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-12-09
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * SPDX-FileCopyrightText: 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintphoto.h"

// Qt includes

#include <QFileInfo>
#include <QPolygon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"
#include "advprintwizard.h"

namespace DigikamGenericPrintCreatorPlugin
{

AdvPrintPhotoSize::AdvPrintPhotoSize()
    : m_label(i18n("Unsupported Paper Size"))
{
}

AdvPrintPhotoSize::AdvPrintPhotoSize(const AdvPrintPhotoSize& other)
    : m_label     (other.m_label),
      m_dpi       (other.m_dpi),
      m_autoRotate(other.m_autoRotate),
      m_layouts   (other.m_layouts),
      m_icon      (other.m_icon)

{
}

// -----------------------------

AdvPrintAdditionalInfo::AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& other)
    : m_unit                (other.m_unit),
      m_printPosition       (other.m_printPosition),
      m_scaleMode           (other.m_scaleMode),
      m_keepRatio           (other.m_keepRatio),
      m_autoRotate          (other.m_autoRotate),
      m_printWidth          (other.m_printWidth),
      m_printHeight         (other.m_printHeight),
      m_enlargeSmallerImages(other.m_enlargeSmallerImages)
{
}

// -----------------------------

AdvPrintCaptionInfo::AdvPrintCaptionInfo(const AdvPrintCaptionInfo& other)
    : m_captionType (other.m_captionType),
      m_captionFont (other.m_captionFont),
      m_captionColor(other.m_captionColor),
      m_captionSize (other.m_captionSize),
      m_captionText (other.m_captionText)
{
}

// -----------------------------

AdvPrintPhoto::AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface)
    : m_thumbnailSize(thumbnailSize),
      m_iface        (iface)
{
}

AdvPrintPhoto::AdvPrintPhoto(const AdvPrintPhoto& other)
    : m_url                 (other.m_url),
      m_thumbnailSize       (other.m_thumbnailSize),
      m_cropRegion          (other.m_cropRegion),
      m_first               (other.m_first),
      m_copies              (other.m_copies),
      m_rotation            (other.m_rotation),
      m_pAddInfo            (nullptr),
      m_pAdvPrintCaptionInfo(nullptr),
      m_iface               (other.m_iface),
      m_thumbnail           (nullptr),
      m_size                (nullptr)
{
    if (other.m_pAddInfo)
    {
        m_pAddInfo = new AdvPrintAdditionalInfo(*other.m_pAddInfo);
    }

    if (other.m_pAdvPrintCaptionInfo)
    {
        m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo(*other.m_pAdvPrintCaptionInfo);
    }
}

AdvPrintPhoto::~AdvPrintPhoto()
{
    delete m_thumbnail;
    delete m_size;
    delete m_pAddInfo;
    delete m_pAdvPrintCaptionInfo;
}

void AdvPrintPhoto::loadInCache()
{
    // Load the thumbnail and size only once.

    delete m_thumbnail;
    DImg photo  = loadPhoto();
    m_thumbnail = new DImg(photo.smoothScale(m_thumbnailSize, m_thumbnailSize, Qt::KeepAspectRatio));

    delete m_size;
    m_size      = new QSize(photo.width(), photo.height());
}

DImg& AdvPrintPhoto::thumbnail()
{
    if (!m_thumbnail)
    {
        loadInCache();
    }

    return *m_thumbnail;
}

DImg AdvPrintPhoto::loadPhoto()
{
    return PreviewLoadThread::loadHighQualitySynchronously(m_url.toLocalFile());
}

QSize& AdvPrintPhoto::size()
{
    if (m_size == nullptr)
    {
        loadInCache();
    }

    return *m_size;
}

int AdvPrintPhoto::width()
{
    return size().width();
}

int AdvPrintPhoto::height()
{
    return size().height();
}

double AdvPrintPhoto::scaleWidth(double unitToInches)
{
    Q_ASSERT(m_pAddInfo != nullptr);

    m_cropRegion = QRect(0, 0,
                         (int)(m_pAddInfo->m_printWidth  * unitToInches),
                         (int)(m_pAddInfo->m_printHeight * unitToInches));

    return (m_pAddInfo->m_printWidth * unitToInches);
}

double AdvPrintPhoto::scaleHeight(double unitToInches)
{
    Q_ASSERT(m_pAddInfo != nullptr);

    m_cropRegion = QRect(
                         0, 0,
                         (int)(m_pAddInfo->m_printWidth  * unitToInches),
                         (int)(m_pAddInfo->m_printHeight * unitToInches)
                        );

    return (m_pAddInfo->m_printHeight * unitToInches);
}

QTransform AdvPrintPhoto::updateCropRegion(int woutlay, int houtlay, bool autoRotate)
{
    QSize thmSize        = thumbnail().size();
    QRect imgRect        = QRect(0, 0, size().width(), size().height());
    bool resetCropRegion = (m_cropRegion == QRect(-1, -1, -1, -1));

    if (resetCropRegion)
    {
        // First, let's see if we should rotate

        if (autoRotate)
        {
            if (
                (m_rotation == 0) &&
                (
                 ((woutlay > houtlay) && (thmSize.height() > thmSize.width())) ||
                 ((houtlay > woutlay) && (thmSize.width()  > thmSize.height()))
                )
               )
            {
                // We will perform a rotation

                m_rotation = 90;
            }
        }
    }
    else
    {
        // Does the crop region need updating (but the image shouldn't be rotated)?

        resetCropRegion = (m_cropRegion == QRect(-2, -2, -2, -2));
    }

    // Rotate the image rectangle.

    QTransform matrix;
    matrix.rotate(m_rotation);
    imgRect = matrix.mapToPolygon(imgRect).boundingRect();
    imgRect.translate((-1)*imgRect.x(), (-1)*imgRect.y());

    // Size the rectangle based on the minimum image dimension.

    int w   = imgRect.width();
    int h   = imgRect.height();

    if (w < h)
    {
        h = AdvPrintWizard::normalizedInt((double)w * ((double)houtlay / (double)woutlay));

        if (h > imgRect.height())
        {
            h = imgRect.height();
            w = AdvPrintWizard::normalizedInt((double)h * ((double)woutlay / (double)houtlay));
        }
    }
    else
    {
        w = AdvPrintWizard::normalizedInt((double)h * ((double)woutlay / (double)houtlay));

        if (w > imgRect.width())
        {
            w = imgRect.width();
            h = AdvPrintWizard::normalizedInt((double)w * ((double)houtlay / (double)woutlay));
        }
    }

    if (resetCropRegion)
    {
        m_cropRegion = QRect(
                             (imgRect.width()  / 2) - (w / 2),
                             (imgRect.height() / 2) - (h / 2),
                             w, h
                            );
    }

    return matrix;
}

} // Namespace DigikamGenericPrintCreatorPlugin
