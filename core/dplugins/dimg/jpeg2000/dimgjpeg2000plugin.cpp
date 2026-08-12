/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : JPEG-2000 DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpeg2000plugin.h"

// Qt includes

#include <QFile>
#include <QImageReader>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimgjpeg2000loader_p.h"
#include "dimgjpeg2000exportsettings.h"

namespace DigikamJPEG2000DImgPlugin
{

DImgJPEG2000Plugin::DImgJPEG2000Plugin(QObject* const parent)
    : DPluginDImg (parent)
{

#if defined JAS_VERSION_MAJOR && JAS_VERSION_MAJOR >= 3

    const auto frms = QImageReader::supportedImageFormats();

    if (frms.contains(QByteArray("jp2")))
    {
        m_useQReader = true;

        return;
    }

    size_t max_mem = jas_get_total_mem_size();

    if (max_mem)
    {
        max_mem = 0.75 * max_mem;
    }
    else
    {
        max_mem = (size_t)1024 * 1024 * 1024;
    }

    jas_conf_clear();
    jas_conf_set_debug_level(0);
    jas_conf_set_multithread(1);
    jas_conf_set_max_mem_usage(max_mem);

    if (!jas_init_library())
    {
        m_initJasper = true;
    }

#else

    m_initJasper = true;

#endif

}

DImgJPEG2000Plugin::~DImgJPEG2000Plugin()
{

#if defined JAS_VERSION_MAJOR && JAS_VERSION_MAJOR >= 3

    if (m_initJasper)
    {
        jas_cleanup_library();
    }

#endif

}

QString DImgJPEG2000Plugin::name() const
{
    return i18nc("@title", "JPEG-2000 loader");
}

QString DImgJPEG2000Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgJPEG2000Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString DImgJPEG2000Plugin::description() const
{
    return i18nc("@info", "An image loader based on Libjasper codec");
}

QString DImgJPEG2000Plugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libjasper codec</para>"
                  "<para>The JPEG (Joint Photographic Experts Group) 2000 standard, finalized in 2001, "
                  "defines a image-coding scheme using state-of-the-art compression techniques based "
                  "on wavelet technology. Its architecture is useful for many diverse applications, "
                  "including image archiving, security systems, digital photography, and medical imaging.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/JPEG_2000'>"
                  "Joint Photographic Experts Group documentation</a> for details.</para>"
    );
}

QString DImgJPEG2000Plugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgJPEG2000Plugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgJPEG2000Plugin::handbookReference() const
{
    return QLatin1String("image-jpeg2000");
}

QList<DPluginAuthor> DImgJPEG2000Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void DImgJPEG2000Plugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QMap<QString, QStringList> DImgJPEG2000Plugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("JP2"), QStringList() << i18nc("@title", "JPEG-2000 image")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );
    map.insert(QLatin1String("JPX"), QStringList() << i18nc("@title", "JPEG-2000 image")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );
    map.insert(QLatin1String("JPC"), QStringList() << i18nc("@title", "JPEG-2000 stream")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );
    map.insert(QLatin1String("J2K"), QStringList() << i18nc("@title", "JPEG-2000 stream")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );
    map.insert(QLatin1String("PGX"), QStringList() << i18nc("@title", "JPEG-2000 verification model")
                                                   << i18nc("@info: can read file format",  "yes")
                                                   << i18nc("@info: can write file format", "yes")
    );

    return map;
}

QString DImgJPEG2000Plugin::loaderName() const
{
    return QLatin1String("JPEG2000");
}

QString DImgJPEG2000Plugin::typeMimes() const
{
    return QLatin1String("JP2 JPX JPC J2K JP2K PGX");
}

int DImgJPEG2000Plugin::canRead(const QFileInfo& fileInfo, bool magic) const
{
    if (!m_initJasper)
    {
        return 0;
    }

    QString filePath = fileInfo.filePath();
    QString format   = fileInfo.suffix().toUpper();

    // First simply check file extension

    if (!magic)
    {
        return ((!format.isEmpty() && typeMimes().contains(format)) ? 10 : 0);
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

    const uchar jp2ID[5] = { 0x6A, 0x50, 0x20, 0x20, 0x0D, };
    const uchar jpcID[2] = { 0xFF, 0x4F };

    if (
        (memcmp(&header.data()[4], &jp2ID, 5) == 0) ||
        (memcmp(header.data(),     &jpcID, 2) == 0)
       )
    {
        return 10;
    }

    return 0;
}

int DImgJPEG2000Plugin::canWrite(const QString& format) const
{
    if (!m_initJasper)
    {
        return 0;
    }

    return (typeMimes().contains(format.toUpper()) ? 10 : 0);
}

DImgLoader* DImgJPEG2000Plugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgJPEG2000Loader(image);
}

DImgLoaderSettings* DImgJPEG2000Plugin::exportWidget(const QString& format) const
{
    if (typeMimes().contains(format.toUpper()) && (m_initJasper || m_useQReader))
    {
        return (new DImgJPEG2000ExportSettings());
    }

    return nullptr;
}

} // namespace DigikamJPEG2000DImgPlugin

#include "moc_dimgjpeg2000plugin.cpp"
