/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : TIFF DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgtiffplugin.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dimgtiffloader.h"
#include "dimgtiffexportsettings.h"

namespace DigikamTIFFDImgPlugin
{

DImgTIFFPlugin::DImgTIFFPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgTIFFPlugin::name() const
{
    return i18nc("@title", "TIFF loader");
}

QString DImgTIFFPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgTIFFPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-tiff"));
}

QString DImgTIFFPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libtiff codec");
}

QString DImgTIFFPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libtiff codec.</para>"
                  "<para>Tagged Image File Format, abbreviated TIFF or TIF, is a computer file format "
                  "for storing raster graphics images, popular among graphic artists, the publishing "
                  "industry, and photographers. TIFF is widely supported by scanning, faxing, "
                  "word processing, optical character recognition, image manipulation, "
                  "desktop publishing, and page-layout applications.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/TIFF'>"
                  "Tagged Image File Format documentation</a> for details.</para>"
    );
}

QString DImgTIFFPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgTIFFPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgTIFFPlugin::handbookReference() const
{
    return QLatin1String("image-tiff");
}

QList<DPluginAuthor> DImgTIFFPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2025"))
            ;
}

void DImgTIFFPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString DImgTIFFPlugin::loaderName() const
{
    return QLatin1String("TIFF");
}

QString DImgTIFFPlugin::typeMimes() const
{
    return QLatin1String("TIF TIFF");
}

QMap<QString, QStringList> DImgTIFFPlugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("TIF"),  QStringList() << i18nc("@title", "Tagged Image Format")
                                                    << i18nc("@info: can read file format",  "yes")
                                                    << i18nc("@info: can write file format", "yes")
    );
    map.insert(QLatin1String("TIFF"), QStringList() << i18nc("@title", "Tagged Image Format")
                                                    << i18nc("@info: can read file format",  "yes")
                                                    << i18nc("@info: can write file format", "yes")
    );

    return map;
}

int DImgTIFFPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
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

    const uchar tiffBigID[4] = { 0x4D, 0x4D, 0x00, 0x2A };
    const uchar tiffLilID[4] = { 0x49, 0x49, 0x2A, 0x00 };

    if (
        (memcmp(header.data(), &tiffBigID, 4) == 0) ||
        (memcmp(header.data(), &tiffLilID, 4) == 0)
       )
    {
        return 10;
    }

    return 0;
}

int DImgTIFFPlugin::canWrite(const QString& format) const
{
    return typeMimes().contains(format.toUpper()) ? 10 : 0;
}

DImgLoader* DImgTIFFPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgTIFFLoader(image);
}

DImgLoaderSettings* DImgTIFFPlugin::exportWidget(const QString& format) const
{
    if (canWrite(format))
    {
        return (new DImgTIFFExportSettings());
    }

    return nullptr;
}

} // namespace DigikamTIFFDImgPlugin

#include "moc_dimgtiffplugin.cpp"
