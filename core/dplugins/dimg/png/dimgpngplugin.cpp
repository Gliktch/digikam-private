/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : PNG DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpngplugin.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dimgpngloader.h"
#include "dimgpngexportsettings.h"

namespace DigikamPNGDImgPlugin
{

DImgPNGPlugin::DImgPNGPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgPNGPlugin::name() const
{
    return i18nc("@title", "PNG loader");
}

QString DImgPNGPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgPNGPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-png"));
}

QString DImgPNGPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libpng codec");
}

QString DImgPNGPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libpng codec.</para>"
                  "<para>Portable Network Graphics (PNG) is a raster-graphics file-format that supports "
                  "lossless data compression. PNG was developed as an improved, non-patented replacement "
                  "for Graphics Interchange Format.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/Portable_Network_Graphics'>"
                  "Portable Network Graphics documentation</a> for details.</para>"
    );
}

QString DImgPNGPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgPNGPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgPNGPlugin::handbookReference() const
{
    return QLatin1String("image-png");
}

QList<DPluginAuthor> DImgPNGPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2025"))
            ;
}

void DImgPNGPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString DImgPNGPlugin::loaderName() const
{
    return QLatin1String("PNG");
}

QString DImgPNGPlugin::typeMimes() const
{
    return QLatin1String("PNG");
}

QMap<QString, QStringList> DImgPNGPlugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("PNG"), QStringList() << i18nc("@title", "Portable Network Graphic")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );

    return map;
}

int DImgPNGPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
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

    const uchar pngID[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    if (memcmp(header.data(), &pngID, 8) == 0)
    {
        return 10;
    }

    return 0;
}

int DImgPNGPlugin::canWrite(const QString& format) const
{
    return (typeMimes().contains(format.toUpper()) ? 10 : 0);
}

DImgLoader* DImgPNGPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgPNGLoader(image);
}

DImgLoaderSettings* DImgPNGPlugin::exportWidget(const QString& format) const
{
    if (canWrite(format))
    {
        return (new DImgPNGExportSettings());
    }

    return nullptr;
}

} // namespace DigikamPNGDImgPlugin

#include "moc_dimgpngplugin.cpp"
