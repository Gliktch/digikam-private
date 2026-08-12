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

#pragma once

// Qt includes

#include <QRect>
#include <QFont>
#include <QColor>
#include <QUrl>
#include <QPointer>
#include <QIcon>
#include <QList>
#include <QSize>
#include <QTransform>

// Local includes

#include "dinfointerface.h"
#include "dimg.h"
#include "advprintsettings.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintPhotoSize
{
public:

    AdvPrintPhotoSize();
    AdvPrintPhotoSize(const AdvPrintPhotoSize& other);
    ~AdvPrintPhotoSize() = default;

public:

    QString       m_label;
    int           m_dpi         = 0;
    bool          m_autoRotate  = false;
    QList<QRect*> m_layouts;    ///< first element is page size
    QIcon         m_icon;
};

// -----------------------------------------------------------

class AdvPrintAdditionalInfo
{
public:

    AdvPrintAdditionalInfo()  = default;
    AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& other);
    ~AdvPrintAdditionalInfo() = default;

public:

    int    m_unit                   = 0;
    int    m_printPosition          = 0;
    int    m_scaleMode              = 0;
    bool   m_keepRatio              = true;
    bool   m_autoRotate             = true;
    double m_printWidth             = 0.0;
    double m_printHeight            = 0.0;
    bool   m_enlargeSmallerImages   = false;
};

// -----------------------------------------------------------

class AdvPrintCaptionInfo
{

public:

    AdvPrintCaptionInfo()  = default;

    /// Copy constructor to get old photo info.
    AdvPrintCaptionInfo(const AdvPrintCaptionInfo& other);
    ~AdvPrintCaptionInfo() = default;

public:

    AdvPrintSettings::CaptionType m_captionType     = AdvPrintSettings::NONE;
    QFont                         m_captionFont     = QFont(QLatin1String("Sans Serif"));
    QColor                        m_captionColor    = Qt::yellow;
    int                           m_captionSize     = 2;
    QString                       m_captionText;
};

// -----------------------------------------------------------

class AdvPrintPhoto
{

public:

    explicit AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface);
    AdvPrintPhoto(const AdvPrintPhoto& other);
    ~AdvPrintPhoto();

    DImg&  thumbnail();
    DImg   loadPhoto();
    int    width();
    int    height();
    QSize& size();

    QTransform updateCropRegion(int woutlay, int houtlay, bool autoRotate);

    double scaleWidth(double unitToInches);
    double scaleHeight(double unitToInches);

public:

    /// Url of original image file.
    QUrl                    m_url;

    /// Thumbnail size in pixels.
    int                     m_thumbnailSize;

    /// Region to crop while print from original image.
    QRect                   m_cropRegion            = QRect(-1, -1, -1, -1);

    /// To get first copy quickly.
    bool                    m_first                 = false;

    /// Number of copies while printing stage.
    int                     m_copies                = 1;

    /// Rotation angle in degrees.
    int                     m_rotation              = 0;

    AdvPrintAdditionalInfo* m_pAddInfo              = nullptr;
    AdvPrintCaptionInfo*    m_pAdvPrintCaptionInfo  = nullptr;
    DInfoInterface*         m_iface                 = nullptr;

private:

    void loadInCache();

private:

    DImg*                   m_thumbnail             = nullptr;
    QSize*                  m_size                  = nullptr;

private:

    /// @note disabled
    AdvPrintPhoto& operator=(const AdvPrintPhoto&) = delete;
};

} // Namespace DigikamGenericPrintCreatorPlugin
