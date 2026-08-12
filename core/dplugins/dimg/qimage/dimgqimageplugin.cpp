/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-20
 * Description : QImage DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgqimageplugin.h"

// Qt includes

#include <QImageReader>
#include <QImageWriter>
#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimgqimageloader.h"
#include "dimgjxlexportsettings.h"
#include "dimgavifexportsettings.h"
#include "dimgwebpexportsettings.h"

namespace DigikamQImageDImgPlugin
{

DImgQImagePlugin::DImgQImagePlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgQImagePlugin::name() const
{
    return i18nc("@title", "QImage loader");
}

QString DImgQImagePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgQImagePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString DImgQImagePlugin::description() const
{
    return i18nc("@info", "An image loader based on QImage plugins");
}

QString DImgQImagePlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using QImage plugins from Qt Framework.</para>"
                  "<para>See <a href='https://doc.qt.io/qt-5/qimage.html#reading-and-writing-image-files'>Qt Framework documentation</a> "
                  "for main native list of format supported.</para>"
                  "<para>See <a href='https://invent.kde.org/frameworks/kimageformats/-/blob/master/README.md'>"
                  "KDE Framework documentation</a> for extended list of formats supported.</para>"
    );
}


QString DImgQImagePlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgQImagePlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgQImagePlugin::handbookReference() const
{
    return QLatin1String("image-others");
}

QList<DPluginAuthor> DImgQImagePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2005"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2025"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2025"))
            ;
}

void DImgQImagePlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString DImgQImagePlugin::loaderName() const
{
    return QLatin1String("QIMAGE");
}

QString DImgQImagePlugin::typeMimes() const
{
    QString ret;
    const auto frms = QImageReader::supportedImageFormats();

    for (const QByteArray& ba : frms)
    {
        ret += QString::fromUtf8("%1 ").arg(QString::fromUtf8(ba).toUpper());
    }

    return ret;
}

int DImgQImagePlugin::canRead(const QFileInfo& fileInfo, bool magic) const
{
    QString filePath = fileInfo.filePath();
    QString format   = fileInfo.suffix().toUpper();

    QString mimeType(QMimeDatabase().mimeTypeForFile(filePath).name());

    // Ignore non image format.

    if (
        mimeType.startsWith(QLatin1String("video/")) ||
        mimeType.startsWith(QLatin1String("audio/"))
       )
    {
        return 0;
    }

    if (!magic)
    {
        const auto frms = QImageReader::supportedImageFormats();

        for (const QByteArray& ba : frms)
        {
            if (QString::fromUtf8(ba).toUpper() == format)
            {
                return 80;
            }
        }

        return 0;
    }

    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    QByteArray readFormat = reader.format();

    if (readFormat == QByteArray("raw"))
    {
        return 0;
    }

    return (QImageReader::supportedImageFormats().contains(readFormat) ? 80 : 0);
}

int DImgQImagePlugin::canWrite(const QString& format) const
{
    const auto frms = QImageWriter::supportedImageFormats();

    for (const QByteArray& ba : frms)
    {
        if (QString::fromUtf8(ba).toUpper() == format.toUpper())
        {
            return 80;
        }
    }

    return 0;
}

DImgLoader* DImgQImagePlugin::loader(DImg* const image, const DRawDecoding&) const
{
    return new DImgQImageLoader(image);
}

DImgLoaderSettings* DImgQImagePlugin::exportWidget(const QString& format) const
{
    if      (format.toUpper() == QLatin1String("JXL"))
    {
        return (new DImgJXLExportSettings());
    }
    else if (format.toUpper() == QLatin1String("AVIF"))
    {
        return (new DImgAVIFExportSettings());
    }
    else if (format.toUpper() == QLatin1String("WEBP"))
    {
        return (new DImgWEBPExportSettings());
    }

    return nullptr;
}

} // namespace DigikamQImageDImgPlugin

#include "moc_dimgqimageplugin.cpp"
