/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QString>

// KDE includes

#include <kconfigskeleton.h>

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryConfig : public KConfigSkeleton
{
    Q_OBJECT

public:

    class EnumFullFormat
    {
    public:

        enum type
        {
            JPEG,
            JPG,
            PNG,
            COUNT
        };
    };

    class EnumThumbnailFormat
    {
    public:

        enum type
        {
            JPEG,
            JPG,
            PNG,
            COUNT
        };
    };

    // Web Browser to use
    enum EnumWebBrowser
    {
        NOBROWSER = 0,
        INTERNAL,
        DESKTOP,
        COUNT
    };

public:

    explicit GalleryConfig(QObject* const parent = nullptr);
    ~GalleryConfig() override = default;

    void setTheme(const QString&);
    QString theme() const;

    void setUseOriginalImageAsFullImage(bool);
    bool useOriginalImageAsFullImage() const;

    void setFullResize(bool);
    bool fullResize() const;

    void setFullSize(int);
    int fullSize() const;

    void setFullFormat(int);
    int fullFormat() const;

    void setFullQuality(int);
    int fullQuality() const;

    void setCopyOriginalImage(bool);
    bool copyOriginalImage() const;

    void setThumbnailSize(int);
    int thumbnailSize() const;

    void setThumbnailFormat(int);
    int thumbnailFormat() const;

    void setThumbnailQuality(int);
    int thumbnailQuality() const;

    void setThumbnailSquare(bool);
    bool thumbnailSquare() const;

    void setDestUrl(const QUrl&);
    QUrl destUrl() const;

    void setOpenInBrowser(int);
    int openInBrowser() const;

    void setImageSelectionTitle(const QString&);
    QString imageSelectionTitle() const;

protected:

    QString    m_theme;
    bool       m_useOriginalImageAsFullImage    = false;
    bool       m_fullResize                     = true;
    int        m_fullSize                       = 1024;
    int        m_fullFormat                     = EnumFullFormat::JPEG;
    int        m_fullQuality                    = 80;
    bool       m_copyOriginalImage              = false;
    int        m_thumbnailSize                  = 120;
    int        m_thumbnailFormat                = EnumThumbnailFormat::JPEG;
    int        m_thumbnailQuality               = 80;
    bool       m_thumbnailSquare                = true;
    QUrl       m_destUrl;
    int        m_openInBrowser                  = EnumWebBrowser::INTERNAL;
    QString    m_imageSelectionTitle;           ///< Gallery title to use for GalleryInfo::ImageGetOption::IMAGES selection.
};

} // namespace DigikamGenericHtmlGalleryPlugin
