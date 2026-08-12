/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "libsinfodlg_p.h"

namespace Digikam
{

void LibsInfoDlg::populateLibraries()
{
    m_libraries = new QTreeWidgetItem(listView(), QStringList() << i18nc("@title", "Libraries"));
    listView()->addTopLevelItem(m_libraries);


    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Qt Framework") <<                   QLatin1String(qVersion()));

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "KDE Framework") <<                  QLatin1String(KXMLGUI_VERSION_STRING));

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibRaw") <<                         DRawDecoder::librawVersion());

#ifdef HAVE_EIGEN3

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Eigen") <<                          QLatin1String(EIGEN3_VERSION_STRING));

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Eigen support") <<                  d->supportedNo());

#endif

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Qt WebEngine version") <<           QLatin1String(QTWEBENGINEWIDGETS_VERSION_STR));


    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Exiv2") <<                          MetaEngine::Exiv2Version());

    ExifToolParser* const parser = new ExifToolParser(this);
    ExifToolParser::ExifToolData parsed;

    if (parser->version())
    {
        parsed            = parser->currentData();
        QString etVersion = parsed.find(QLatin1String("VERSION_STRING")).value()[0].toString();
        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc(Private::CONTEXT, "ExifTool") <<                   etVersion);
    }
    else
    {
        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc(Private::CONTEXT, "ExifTool support") <<           d->supportedNo());
    }

#ifdef HAVE_LENSFUN

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LensFun") <<                        LensFunIface::lensFunVersion());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "LensFun support") <<                d->supportedNo());

#endif

#ifdef HAVE_IMAGE_MAGICK

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "ImageMagick codecs") <<             QLatin1String(MagickLibVersionText));

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "ImageMagick codecs support") <<     d->supportedNo());

#endif

#ifdef HAVE_HEIF

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibHEIF") <<                        QLatin1String(LIBHEIF_VERSION));
    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "HEIF reading support") <<           d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "HEIF reading support") <<           d->supportedNo());

#endif

#ifdef HAVE_X265

    const x265_api* const x265api = x265_api_get(0);

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Libx265") <<             (x265api ? QLatin1String(x265api->version_str)
                                                                          : i18nc("@info: libx265 version", "Unknown")));
    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "HEIF writing support") <<           d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "HEIF Writing support") <<           d->supportedNo());

#endif

    QString tiffver = QLatin1String(TIFFLIB_VERSION_STR);
    tiffver         = tiffver.left(tiffver.indexOf(QLatin1Char('\n')));
    tiffver         = tiffver.section(QLatin1Char(' '), 2, 2);
    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibTIFF") <<                        tiffver);

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibPNG") <<                         QLatin1String(PNG_LIBPNG_VER_STRING));

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibJPEG") <<                        QString::number(JPEG_LIB_VERSION));

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibCImg") <<                        GreycstorationFilter::cimgVersionString());

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibLCMS") <<                        QString::number(LCMS_VERSION));

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibPGF") <<                         PGFUtils::libPGFVersion());

#ifdef HAVE_JXL

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "XMP SDK") <<                        DNGWriter::xmpSdkVersion());

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "DNG SDK") <<                        DNGWriter::dngSdkVersion());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "DNG support") <<                    d->supportedNo());

#endif

#ifdef HAVE_JASPER

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "LibJasper") <<                      QLatin1String(jas_getversion()));

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "LibJasper support") <<              d->supportedNo());

#endif

#ifdef HAVE_GEOLOCATION

    new QTreeWidgetItem(m_libraries, QStringList() <<
                        i18nc(Private::CONTEXT, "Geolocation support") <<            d->supportedYes());

#else

    new QTreeWidgetItem(m_features, QStringList() <<
                        i18nc(Private::CONTEXT, "Geolocation support") <<            d->supportedNo());

#endif

}

} // namespace Digikam
