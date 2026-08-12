/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : RAW DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgrawplugin.h"

// C++ includes

#include <cstdio>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimgrawloader.h"
#include "drawdecoder.h"
#include "drawfiles.h"

namespace DigikamRAWDImgPlugin
{

DImgRAWPlugin::DImgRAWPlugin(QObject* const parent)
    : DPluginDImg(parent)
{
}

QString DImgRAWPlugin::name() const
{
    return i18nc("@title", "RAW loader");
}

QString DImgRAWPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DImgRAWPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString DImgRAWPlugin::description() const
{
    return i18nc("@info", "An image loader based on Libraw codec");
}

QString DImgRAWPlugin::details() const
{
    return xi18nc("@info", "<para>This plugin allows users to load and save image using Libraw codec.</para>"
                  "<para>A camera raw image file contains minimally processed data from the image sensor "
                  "of either a digital camera, a motion picture film scanner, or other image scanner. "
                  "Raw files are not yet processed and therefore are not ready to be printed or edited "
                  "with a bitmap graphics editor. RAW are processed by a raw converter in a wide-gamut "
                  "internal color space where precise adjustments can be made before conversion to a TIFF "
                  "or JPEG for storage, printing, or further manipulation. This often encodes the image in "
                  "a device-dependent color space.</para>"
                  "<para>See <a href='https://en.wikipedia.org/wiki/Raw_image_format'>"
                  "Raw Image File documentation</a> for details.</para>"
    );
}

QString DImgRAWPlugin::handbookSection() const
{
    return QLatin1String("supported_materials");
}

QString DImgRAWPlugin::handbookChapter() const
{
    return QLatin1String("image_formats");
}

QString DImgRAWPlugin::handbookReference() const
{
    return QLatin1String("image-raw");
}

QList<DPluginAuthor> DImgRAWPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2005-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2025"))
            ;
}

void DImgRAWPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QMap<QString, QStringList> DImgRAWPlugin::extraAboutData() const
{
    QMap<QString, QString> rawMap = s_rawFileExtensionsdWithDesc();
    QMap<QString, QStringList> ret;

    for (QMap<QString, QString>::const_iterator it = rawMap.constBegin() ; it != rawMap.constEnd() ; ++it)
    {
        ret.insert(it.key(), QStringList() << it.value()
                                           << i18nc("@info: can read file format",  "yes")
                                           << i18nc("@info: can write file format", "no")
        );
    }

    return ret;
}

QString DImgRAWPlugin::loaderName() const
{
    return QLatin1String("RAW");
}

QString DImgRAWPlugin::typeMimes() const
{
    return QString(DRawDecoder::rawFiles()).toUpper().remove(QLatin1String("*."));
}

int DImgRAWPlugin::canRead(const QFileInfo& fileInfo, bool magic) const
{
    if (!magic)
    {
        QString rawFilesExt = DRawDecoder::rawFiles();
        QString format      = fileInfo.suffix().toUpper();

        return ((!format.isEmpty() && rawFilesExt.toUpper().contains(format)) ? 10 : 0);
    }

    return 0;
}

int DImgRAWPlugin::canWrite(const QString& /*format*/) const
{
    // NOTE: Raw are read only files.

    return 0;
}

DImgLoader* DImgRAWPlugin::loader(DImg* const image, const DRawDecoding& rawSettings) const
{
    return (new DImgRAWLoader(image, rawSettings));
}

DImgLoaderSettings* DImgRAWPlugin::exportWidget(const QString& format) const
{
    Q_UNUSED(format);

    // NOTE: RAW are read only.

    return nullptr;
}

} // namespace DigikamRAWDImgPlugin

#include "moc_dimgrawplugin.cpp"
