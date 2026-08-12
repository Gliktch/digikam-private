/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : PGF DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpgfplugin.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dimgpgfloader.h"
#include "dimgpgfexportsettings.h"

namespace DigikamPGFDImgPlugin
{

DImgPGFPlugin::DImgPGFPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgPGFPlugin::name() const
{
    return i18nc("@title", "PGF loader");
}

QString DImgPGFPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgPGFPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString DImgPGFPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libpgf codec");
}

QString DImgPGFPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgPGFPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgPGFPlugin::handbookReference() const
{
    return QLatin1String("image-pgf");
}

QString DImgPGFPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libpgf codec.</para>"
                  "<para>The Progressive Graphics File (PGF) is an efficient image file format, "
                  "that is based on a fast, discrete wavelet transform with progressive coding "
                  "features. PGF can be used for lossless and lossy compression. It is most suitable "
                  "for natural images. PGF can be used as a very efficient and fast replacement of JPEG-2000.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/Progressive_Graphics_File'>"
                  "Progressive Graphics File documentation</a> for details.</para>"
    );
}

QList<DPluginAuthor> DImgPGFPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2025"))
            ;
}

void DImgPGFPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QMap<QString, QStringList> DImgPGFPlugin::extraAboutData() const
{
    QMap<QString, QStringList> map;
    map.insert(QLatin1String("PGF"),
               QStringList() << i18nc("@title", "Progressive Graphics File")
                             << i18nc("@info: can read file format",  "yes")
                             << i18nc("@info: can write file format", "yes")
    );

    return map;
}

QString DImgPGFPlugin::loaderName() const
{
    return QLatin1String("PGF");
}

QString DImgPGFPlugin::typeMimes() const
{
    return QLatin1String("PGF");
}

int DImgPGFPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
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

    const uchar pgfID[3] = { 0x50, 0x47, 0x46 };

    if (memcmp(header.data(), &pgfID, 3) == 0)
    {
        return 10;
    }

    return 0;
}

int DImgPGFPlugin::canWrite(const QString& format) const
{
    return typeMimes().contains(format.toUpper()) ? 10 : 0;
}

DImgLoader* DImgPGFPlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgPGFLoader(image);
}

DImgLoaderSettings* DImgPGFPlugin::exportWidget(const QString& format) const
{
    if (canWrite(format))
    {
        return (new DImgPGFExportSettings());
    }

    return nullptr;
}

} // namespace DigikamPGFDImgPlugin

#include "moc_dimgpgfplugin.cpp"
