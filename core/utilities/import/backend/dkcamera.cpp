/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : abstract camera interface class
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dkcamera.h"

// Local includes

#include "applicationsettings.h"
#include "dmetadata.h"
#include "digikam_debug.h"

namespace Digikam
{

DKCamera::DKCamera(const QString& title, const QString& model, const QString& port, const QString& path)
    : QObject                       (),
      m_model                       (model),
      m_port                        (port),
      m_path                        (path),
      m_title                       (title)
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();
    m_imageFilter                             = settings->getImageFileFilter().toLower().remove(QLatin1String("*.")).split(QLatin1Char(' '));
    m_movieFilter                             = settings->getMovieFileFilter().toLower().remove(QLatin1String("*.")).split(QLatin1Char(' '));
    m_audioFilter                             = settings->getAudioFileFilter().toLower().remove(QLatin1String("*.")).split(QLatin1Char(' '));
    m_rawFilter                               = settings->getRawFileFilter().toLower().remove(QLatin1String("*.")).split(QLatin1Char(' '));
}

QString DKCamera::title() const
{
    return m_title;
}

QString DKCamera::model() const
{
    return m_model;
}

QString DKCamera::port() const
{
    if ((m_port.length() == 21) && m_port.startsWith(QLatin1String("ptpip:")))
    {
        QString portid = QString::fromLatin1("ptpip:%1.%2.%3.%4")
                                  .arg(m_port.mid( 6, 3).toInt())
                                  .arg(m_port.mid(10, 3).toInt())
                                  .arg(m_port.mid(14, 3).toInt())
                                  .arg(m_port.mid(18, 3).toInt());

        return portid;
    }

    return m_port;
}

QString DKCamera::path() const
{
    return m_path;
}

QString DKCamera::uuid() const
{
    return m_uuid;
}

bool DKCamera::thumbnailSupport() const
{
    return m_thumbnailSupport;
}

bool DKCamera::deleteSupport() const
{
    return m_deleteSupport;
}

bool DKCamera::uploadSupport() const
{
    return m_uploadSupport;
}

bool DKCamera::mkDirSupport() const
{
    return m_mkDirSupport;
}

bool DKCamera::delDirSupport() const
{
    return m_delDirSupport;
}

bool DKCamera::captureImageSupport() const
{
    return m_captureImageSupport;
}

bool DKCamera::captureImagePreviewSupport() const
{
    return m_captureImagePreviewSupport;
}

QString DKCamera::mimeType(const QString& fileext) const
{
    if (fileext.isEmpty())
    {
        return QString();
    }

    QString ext = fileext;
    QString mime;

    // Massage known variations of known mimetypes

    if      ((ext == QLatin1String("jpg")) || (ext == QLatin1String("jpe")))
    {
        ext = QLatin1String("jpeg");
    }
    else if (ext == QLatin1String("tif"))
    {
        ext = QLatin1String("tiff");
    }

    if      (m_rawFilter.contains(ext))
    {
        mime = QLatin1String("image/x-raw");
    }
    else if (m_imageFilter.contains(ext))
    {
        mime = QLatin1String("image/") + ext;
    }
    else if (m_movieFilter.contains(ext))
    {
        mime = QLatin1String("video/") + ext;
    }
    else if (m_audioFilter.contains(ext))
    {
        mime = QLatin1String("audio/") + ext;
    }

    qCDebug(DIGIKAM_IMPORTUI_LOG) << "File type:" << ext << "Mime:" << mime;

    return mime;
}

void DKCamera::fillItemInfoFromMetadata(CamItemInfo& info, const DMetadata& meta) const
{
    QSize dims     = meta.getItemDimensions();
    info.ctime     = meta.getItemDateTime();

    // NOTE: see bug #246401 to sort based on milliseconds for items taken quickly.

    if (!info.ctime.isNull())
    {
        info.ctime.setTime(info.ctime.time().addMSecs(meta.getMSecsInfo()));
    }

    info.width     = dims.width();
    info.height    = dims.height();
    info.photoInfo = meta.getPhotographInformation();
}

void DKCamera::printSupportedFeatures()
{
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Supported features for" << title();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Thumbnails:" << thumbnailSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Delete:" << deleteSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Delete dir:" << delDirSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Upload:" << uploadSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Mkdir:" << mkDirSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Image capture:" << captureImageSupport();
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "  Image capture preview (liveview):" << captureImagePreviewSupport();
}

} // namespace Digikam

#include "moc_dkcamera.cpp"
