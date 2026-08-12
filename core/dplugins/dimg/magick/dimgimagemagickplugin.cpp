/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-21
 * Description : Magick DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgimagemagickplugin.h"

// Qt includes

#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Image Magick includes

// Pragma directives to reduce warnings from ImageMagick header files.
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wignored-qualifiers"
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wignored-qualifiers"
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#include <Magick++.h>

#if MagickLibVersion < 0x700
#   include <magick/magick.h>
#endif

using namespace Magick;
using namespace MagickCore;

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimgimagemagickloader.h"

namespace DigikamImageMagickDImgPlugin
{

DImgImageMagickPlugin::DImgImageMagickPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
    MagickCoreGenesis((char*)nullptr, MagickFalse);

    m_readFormats  = decoderFormats();
    m_writeFormats = encoderFormats();
}

void DImgImageMagickPlugin::cleanUp()
{
    MagickCoreTerminus();
}

QString DImgImageMagickPlugin::name() const
{
    return i18nc("@title", "ImageMagick loader");
}

QString DImgImageMagickPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgImageMagickPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString DImgImageMagickPlugin::description() const
{
    return i18nc("@info", "An image loader based on ImageMagick coders");
}

QString DImgImageMagickPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using ImageMagick coders.</para>"
                  "<para>ImageMagick is a free and open-source software suite for converting raster image and vector image files. "
                  "It can read and write over 200 image file formats.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/ImageMagick'>ImageMagick documentation</a> for details.</para>"
    );
}

QString DImgImageMagickPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgImageMagickPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgImageMagickPlugin::handbookReference() const
{
    return QLatin1String("image-others");
}

QList<DPluginAuthor> DImgImageMagickPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2023"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void DImgImageMagickPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QMap<QString, QStringList> DImgImageMagickPlugin::extraAboutData() const
{
    QString mimes = typeMimes();
    QMap<QString, QStringList> map;

    try
    {
        ExceptionInfo ex          = *AcquireExceptionInfo();
        size_t n                  = 0;
        const MagickInfo** inflst = GetMagickInfoList("*", &n, &ex);

        if (!inflst)
        {
            qCWarning(DIGIKAM_DIMG_LOG_MAGICK) << "ImageMagick coders list is null!";

            return QMap<QString, QStringList>();
        }

        for (uint i = 0 ; i < n ; ++i)
        {
            const MagickInfo* inf = inflst[i];

            if (inf)
            {
                QString mod =

#if (MagickLibVersion >= 0x69A && defined(magick_module))

                    QString::fromLatin1(inf->magick_module).toUpper();

#else

                    QString::fromLatin1(inf->module).toUpper();

#endif

                if (mimes.contains(mod))
                {
                    map.insert(mod,
                               QStringList() << QLatin1String(inf->description)
                                             << (inf->decoder ?
                                                 i18nc("@info: can read file format",     "yes") :
                                                 i18nc("@info: cannot read file format",  "no"))
                                             << (inf->encoder ?
                                                 i18nc("@info: can write file format",    "yes") :
                                                 i18nc("@info: cannot write file format", "no"))

                    );
                }
            }
        }

        free(inflst);
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagickInfo exception:" << error.what();

        return QMap<QString, QStringList>();
    }

    return map;
}

QString DImgImageMagickPlugin::loaderName() const
{
    return QLatin1String("IMAGEMAGICK");
}

QString DImgImageMagickPlugin::typeMimes() const
{
    QStringList formats = m_readFormats;
    formats.sort();

    QString ret;

    for (const QString& str : std::as_const(formats))
    {
        if (!ret.contains(str))
        {
            ret += QString::fromUtf8("%1 ").arg(str.toUpper());
        }
    }

    return ret;
}

int DImgImageMagickPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
{
    QString filePath = fileInfo.filePath();
    QString format   = fileInfo.suffix().toUpper();

    if (!magic)
    {
        QString mimeType(QMimeDatabase().mimeTypeForFile(filePath).name());

        // Ignore non image format.

        if (
            mimeType.startsWith(QLatin1String("video/")) ||
            mimeType.startsWith(QLatin1String("audio/"))
           )
        {
            return 0;
        }

        if (!format.isEmpty() && m_readFormats.contains(format))
        {
            return 90;
        }
    }

    return 0;
}

int DImgImageMagickPlugin::canWrite(const QString& format) const
{
    if (m_writeFormats.contains(format.toUpper()))
    {
        return 90;
    }

    return 0;
}

DImgLoader* DImgImageMagickPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgImageMagickLoader(image);
}

QStringList DImgImageMagickPlugin::decoderFormats() const
{
    QStringList formats;

    try
    {
        ExceptionInfo ex          = *AcquireExceptionInfo();
        size_t n                  = 0;
        const MagickInfo** inflst = GetMagickInfoList("*", &n, &ex);

        if (!inflst)
        {
            qWarning() << "ImageMagick coders list is null!";
            return QStringList();
        }

        for (uint i = 0 ; i < n ; ++i)
        {
            const MagickInfo* inf = inflst[i];

            if (inf && inf->decoder)
            {

#if (MagickLibVersion >= 0x69A && defined(magick_module))

                formats.append(QString::fromLatin1(inf->magick_module).toUpper());

#else

                formats.append(QString::fromLatin1(inf->module).toUpper());

#endif

            }
        }

        free(inflst);
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagickInfo exception:" << error.what();

        return QStringList();
    }

    if (formats.contains(QLatin1String("JPEG")))
    {
        formats.append(QLatin1String("JPG"));
        formats.append(QLatin1String("JPE"));
    }

    if (formats.contains(QLatin1String("FITS")))
    {
        formats.append(QLatin1String("FTS"));
        formats.append(QLatin1String("FIT"));
    }

    // Remove known formats that are not stable.

    formats.removeAll(QLatin1String("XCF"));

    return formats;
}

QStringList DImgImageMagickPlugin::encoderFormats() const
{
    QStringList formats;

    try
    {
        ExceptionInfo ex          = *AcquireExceptionInfo();
        size_t n                  = 0;
        const MagickInfo** inflst = GetMagickInfoList("*", &n, &ex);

        if (!inflst)
        {
            qWarning() << "ImageMagick coders list is null!";

            return QStringList();
        }

        for (uint i = 0 ; i < n ; ++i)
        {
            const MagickInfo* inf = inflst[i];

            if (inf && inf->encoder)
            {

#if (MagickLibVersion >= 0x69A && defined(magick_module))

                formats.append(QString::fromLatin1(inf->magick_module).toUpper());

#else

                formats.append(QString::fromLatin1(inf->module).toUpper());

#endif

            }
        }

        free(inflst);
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagickInfo exception:" << error.what();

        return QStringList();
    }

    if (formats.contains(QLatin1String("FITS")))
    {
        formats.append(QLatin1String("FTS"));
    }

    return formats;
}

DImgLoaderSettings* DImgImageMagickPlugin::exportWidget(const QString& format) const
{
    Q_UNUSED(format);

    return nullptr;
}

} // namespace DigikamImageMagickDImgPlugin

#include "moc_dimgimagemagickplugin.cpp"
