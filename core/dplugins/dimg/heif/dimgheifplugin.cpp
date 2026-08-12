/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : HEIF DImg plugin.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgheifplugin.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dimgheifloader.h"
#include "dimgheifexportsettings.h"

namespace DigikamHEIFDImgPlugin
{

DImgHEIFPlugin::DImgHEIFPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgHEIFPlugin::name() const
{
    return i18nc("@title", "HEIF loader");
}

QString DImgHEIFPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgHEIFPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString DImgHEIFPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libheif codec");
}

QString DImgHEIFPlugin::details() const
{
    QString x265Notice = i18nc("@info", "This library is not present on your system.");
    Q_UNUSED(x265Notice);

#ifdef HAVE_X265

    int depth = DImgHEIFLoader::x265MaxBitsDepth();

    if (depth != -1)
    {
        x265Notice = i18nc("@info", "This library is available on your system with a maximum color depth "
                           "support of %1 bits.", depth);
    }
    else
    {
        x265Notice = i18nc("@info", "This library is available on your system but is not able to encode "
                           "image with a suitable color depth.");
    }

#endif

    return xi18nc("@info",
                  "<para>This plugin allows users to load and save image using Libheif codec.</para>"
                  "<para>High Efficiency Image File Format (HEIF), also known as High Efficiency Image Coding (HEIC), "
                  "is a file format for individual images and image sequences. It was developed by the "
                  "Moving Picture Experts Group (MPEG) and it claims that twice as much information can be "
                  "stored in a HEIF image as in a JPEG image of the same size, resulting in a better quality image. "
                  "HEIF also supports animation, and is capable of storing more information than an animated GIF "
                  "at a small fraction of the size.</para>"
                  "<para>Encoding HEIC is relevant of optional libde265 codec. %1</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/High_Efficiency_Image_File_Format'>"
                  "High Efficiency Image File Format</a> for details.</para>", x265Notice);
}

QString DImgHEIFPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgHEIFPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgHEIFPlugin::handbookReference() const
{
    return QLatin1String("image-heif");
}

QList<DPluginAuthor> DImgHEIFPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2024"))
            ;
}

void DImgHEIFPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QMap<QString, QStringList> DImgHEIFPlugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("HEIC"),
               QStringList() << i18nc("@title", "High efficiency image coding")
                             << i18nc("@info: can read file format", "yes")

#ifdef HAVE_X265

                             << i18nc("@info: can write file format", "yes")

#else

                             << i18nc("@info: cannot write file format", "no")

#endif

    );

    map.insert(QLatin1String("HEIF"),
               QStringList() << i18nc("@title", "High efficiency image coding")
                             << i18nc("@info: can read file format", "yes")

#ifdef HAVE_X265

                             << i18nc("@info: can write file format", "yes")

#else

                             << i18nc("@info: cannot write file format", "no")

#endif

    );

    return map;
}

bool DImgHEIFPlugin::previewSupported() const
{
    return true;
}

QString DImgHEIFPlugin::loaderName() const
{
    return QLatin1String("HEIF");
}

QString DImgHEIFPlugin::typeMimes() const
{
    return QLatin1String("HEIC HEIF HIF");
}

int DImgHEIFPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
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

    const qint64 headerLen = 12;

    QByteArray header(headerLen, '\0');

    if (file.read(const_cast<char*>(header.data()), headerLen) != headerLen)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to read header of file " << filePath;

        return 0;
    }

    if (
        (memcmp(&header.data()[4], "ftypheic", 8) == 0) ||
        (memcmp(&header.data()[4], "ftypheix", 8) == 0) ||
        (memcmp(&header.data()[4], "ftypmif1", 8) == 0)
       )
    {
        return 10;
    }

    return 0;
}

int DImgHEIFPlugin::canWrite(const QString& format) const
{

#ifdef HAVE_X265

    return (typeMimes().contains(format.toUpper()) ? 10 : 0);

#else

    Q_UNUSED(format);

    return 0;

#endif

}

DImgLoader* DImgHEIFPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return (new DImgHEIFLoader(image));
}

DImgLoaderSettings* DImgHEIFPlugin::exportWidget(const QString& format) const
{
    if (canWrite(format))
    {
        return (new DImgHEIFExportSettings());
    }

    return nullptr;
}

} // namespace DigikamHEIFDImgPlugin

#include "moc_dimgheifplugin.cpp"
