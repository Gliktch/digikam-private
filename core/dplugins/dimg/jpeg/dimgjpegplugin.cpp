/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-21
 * Description : JPEG DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpegplugin.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dimgjpegloader.h"
#include "dimgjpegexportsettings.h"

namespace DigikamJPEGDImgPlugin
{

DImgJPEGPlugin::DImgJPEGPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgJPEGPlugin::name() const
{
    return i18nc("@title", "JPEG loader");
}

QString DImgJPEGPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgJPEGPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString DImgJPEGPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libjpeg codec");
}

QString DImgJPEGPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libjpeg codec.</para>"
                  "<para>Joint Photographic Experts Group (JPEG) is a commonly used method of lossy "
                  "compression for digital images, particularly for those images produced by "
                  "digital photography. The degree of compression can be adjusted, allowing "
                  "a selectable tradeoff between storage size and image quality.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/JPEG'>"
                  "Joint Photographic Experts Group documentation</a> for details.</para>"
    );
}

QString DImgJPEGPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgJPEGPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgJPEGPlugin::handbookReference() const
{
    return QLatin1String("image-jpeg");
}

QList<DPluginAuthor> DImgJPEGPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void DImgJPEGPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString DImgJPEGPlugin::loaderName() const
{
    return QLatin1String("JPEG");
}

QString DImgJPEGPlugin::typeMimes() const
{
    return QLatin1String("JPG JPEG JPE");
}

QMap<QString, QStringList> DImgJPEGPlugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("JPG"),  QStringList() << i18nc("@title", "JPEG image")
                                                    << i18nc("@info: can read file format",  "yes")
                                                    << i18nc("@info: can write file format", "yes")

    );
    map.insert(QLatin1String("JPEG"), QStringList() << i18nc("@title", "JPEG image")
                                                    << i18nc("@info: can read file format",  "yes")
                                                    << i18nc("@info: can write file format", "yes")

    );
    map.insert(QLatin1String("JPE"),  QStringList() << i18nc("@title", "JPEG image")
                                                    << i18nc("@info: can read file format",  "yes")
                                                    << i18nc("@info: can write file format", "yes")

    );

    return map;
}

int DImgJPEGPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
{
    QString filePath = fileInfo.filePath();
    QString format   = fileInfo.suffix().toUpper();

    // First simply check file extension

    if (!magic)
    {
        return (!format.isEmpty() && typeMimes().contains(format)) ? 10 : 0;
    }

    // In second, we trying to parse file header.

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to open file " << filePath;

        return 0;
    }

    const qint64 headerLen = 9;

    QByteArray header(headerLen, '\0');

    if (file.read(const_cast<char*>(header.data()), headerLen) != headerLen)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to read header of file " << filePath;

        return 0;
    }

    const uchar jpegID[2] = { 0xFF, 0xD8 };

    if (memcmp(header.data(), &jpegID, 2) == 0)
    {
        return 10;
    }

    return 0;
}

int DImgJPEGPlugin::canWrite(const QString& format) const
{
    return typeMimes().contains(format.toUpper()) ? 10 : 0;
}

DImgLoader* DImgJPEGPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgJPEGLoader(image);
}

DImgLoaderSettings* DImgJPEGPlugin::exportWidget(const QString& format) const
{
    if (canWrite(format))
    {
        return (new DImgJPEGExportSettings());
    }

    return nullptr;
}

} // namespace DigikamJPEGDImgPlugin

#include "moc_dimgjpegplugin.cpp"
