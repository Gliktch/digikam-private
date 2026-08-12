/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-23
 * Description : item metadata interface - ImageMagick helpers.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QFileInfo>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "drawdecoder.h"

// ImageMagick includes

#ifdef HAVE_IMAGE_MAGICK

// Pragma directives to reduce warnings from ImageMagick header files.
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wignored-qualifiers"
#       pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wignored-qualifiers"
#       pragma clang diagnostic ignored "-Wkeyword-macro"
#   endif

#   include <Magick++.h>

using namespace Magick;
using namespace MagickCore;

// Restore warnings
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif // HAVE_IMAGE_MAGICK

namespace Digikam
{

bool DMetadata::loadUsingImageMagick(const QString& filePath)
{
    bool ret = false;

#ifdef HAVE_IMAGE_MAGICK

    QFileInfo fileInfo(filePath);

    // Ignore null file size to prevent IM crash. See bug #457061

    if (fileInfo.size() == 0)
    {
        return ret;
    }

    QString rawFilesExt  = DRawDecoder::rawFiles();
    QString ext          = fileInfo.suffix().toUpper();

    if (
        !fileInfo.exists() || ext.isEmpty() ||
        rawFilesExt.toUpper().contains(ext) ||    // Ignore RAW files
        (ext == QLatin1String("HEIF"))      ||    // Ignore HEIF files
        (ext == QLatin1String("HEIC"))      ||    // Ignore HEIC files
        (ext == QLatin1String("HIF"))       ||    // Ignore HIF files
        (ext == QLatin1String("XCF"))       ||    // Ignore XCF files
        (ext == QLatin1String("SVG"))       ||    // Ignore SVG files
        (ext == QLatin1String("PDF"))             // Ignore PDF files
       )
    {
        return false;
    }

    MagickCore::ImageInfo* image_info = nullptr;
    ExceptionInfo ex                  = *AcquireExceptionInfo();

    // Syntax description of IM percent escape formats: https://imagemagick.org/script/escape.php
    // WARNING: this string is limited to 1024 characters internally in Image Magick.
    //          Over this size, this will corrupt memory at run time in Image Magick core.
    //          It's recommended to use reduced option forms in favor of long versions.

    const QString filters             = QLatin1String
                                        (
                                            // Standard XMP namespaces                                 Exiv2 Type   IM long option      Exiftool tags

                                            "Xmp.tiff.ImageLength=%h\n"                             // Text         %[height]           MIFF.rows
                                            "Xmp.tiff.ImageWidth=%w\n"                              // Text         %[width]            MIFF.columns
                                            "Xmp.tiff.Compression=%C\n"                             // Text         %[compression]      MIFF.compression
                                            "Xmp.tiff.BitsPerSample=%[bit-depth]\n"                 // Seq          -                   -
                                            "Xmp.tiff.ImageDescription=%[caption]\n"                // LangAlt      -                   -
                                            "Xmp.tiff.Orientation=%[orientation]\n"                 // Text         -                   -
                                            "Xmp.tiff.DateTime=%[date:create]\n"                    // Text         -                   -
                                            "Xmp.tiff.XResolution=%x\n"                             // Text         %[resolution.x]     MIFF.resolution
                                            "Xmp.tiff.YResolution=%y\n"                             // Text         %[resolution.y]     MIFF.resolution
                                            "Xmp.tiff.ResolutionUnit=%U\n"                          // Text         %[units]            -
                                            "Xmp.exif.UserComment=%c\n"                             // LangAlt      %[comment]          -
                                            "Xmp.exif.ColorSpace=%r\n"                              // Text         %[colorspace]       MIFF.colorspace
                                            "Xmp.exifEX.Gamma=%[gamma]\n"                           // Text         -                   MIFF.gamma
                                            "Xmp.xmp.ModifyDate=%[date:modify]\n"                   // Text         -                   -
                                            "Xmp.xmp.Label=%l\n"                                    // Text         %[label]            MIFF.label

                                            // ImageMagick Attributes namespace.
                                            // See Exiftool MIFF namespace for details: https://exiftool.org/TagNames/MIFF.html

                                            "Xmp.MIFF.Version=%[version]\n"                         // Text         -                   -
                                            "Xmp.MIFF.Copyright=%[copyright]\n"                     // Text         -                   -
                                            "Xmp.MIFF.BaseName=%t\n"                                // Text         %[basename]         -
                                            "Xmp.MIFF.Extension=%e\n"                               // Text         %[extension]        -
                                            "Xmp.MIFF.Codec=%m\n"                                   // Text         %[magick]           -
                                            "Xmp.MIFF.Channels=%[channels]\n"                       // Text         -                   -
                                            "Xmp.MIFF.Interlace=%[interlace]\n"                     // Text         -                   -
                                            "Xmp.MIFF.Transparency=%A\n"                            // Text         -                   -
                                            "Xmp.MIFF.Profiles=%[profiles]\n"                       // Text         -                   -
                                            "Xmp.MIFF.ProfileICC=%[profile:icc]\n"                  // Text         -                   MIFF.profile-icc
                                            "Xmp.MIFF.ProfileICM=%[profile:icm]\n"                  // Text         -                   -
                                            "Xmp.MIFF.XPrintSize=%[printsize.x]\n"                  // Text         -                   -
                                            "Xmp.MIFF.YPrintSize=%[printsize.y]\n"                  // Text         -                   -
                                            "Xmp.MIFF.Size=%B\n"                                    // Text         %[size]             -
                                            "Xmp.MIFF.Quality=%Q\n"                                 // Text         %[quality]          -
                                            "Xmp.MIFF.Rendering=%[rendering-intent]\n"              // Text                             MIFF.rendering-intent
                                            "Xmp.MIFF.Scene=%n\n"                                   // Text         %[scene]            MIFF.Scene
/*
   NOTE: values calculated which introduce non negligible time latency:

                                            %k                     (not specified in IM doc)        // Text         %[colors]           MIFF.colors
                                            %[entropy]             (specified as CALCULATED in doc) // Text                             -
                                            %[kurtosis]            (specified as CALCULATED in doc) // Text                             -
                                            %[max]                 (specified as CALCULATED in doc) // Text                             -
                                            %[mean]                (specified as CALCULATED in doc) // Text                             -
                                            %[median]              (specified as CALCULATED in doc) // Text                             -
                                            %[min]                 (specified as CALCULATED in doc) // Text                             -
                                            %[opaque]              (specified as CALCULATED in doc) // Text                             -
                                            %[skewness]            (specified as CALCULATED in doc) // Text                             -
                                            %[standard-deviation]  (specified as CALCULATED in doc) // Text                             -
                                            %[type]                (specified as CALCULATED in doc) // Text                             -
                                            %#                     (specified as CALCULATED in doc) // Text         -                   MIFF.signature
                                            %@                     (specified as CALCULATED in doc) // Text         %[bounding-box]     -
*/

                                            // ImageMagick Properties namespace (MIFP)

                                            "%[*]\n"                                                // Text         -                   -
                                        );

    try
    {
        if (filters.size() >= 1024)
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Size of percent escape format passed to Image Magick interface"
                                                 "is largest than 1024 bytes and metadata cannot be parsed!";
            return ret;
        }

        // Allocate metadata container for IM.

        const int msize     = 256;                  // Number of internal metadata entries prepared for IM.
        char** metadata     = new char*[msize];

        for (int i = 0 ; i < msize ; ++i)
        {
            // NOTE: prepare metadata items with null pointers which will be allocated with malloc on demand by ImageMagick.

            metadata[i] = nullptr;
        }

        // Prepare image info for IM isentification

        image_info          = CloneImageInfo((ImageInfo*)nullptr);
        qstrncpy(image_info->filename, filePath.toLatin1().constData(), sizeof(image_info->filename));
        image_info->ping    = MagickTrue;
        image_info->verbose = MagickTrue;
        image_info->debug   = MagickTrue;

        // Insert percent escape format for IM identification call.

        int identargc       = 4;                             // Number or arguments passed to IM identification call.
        char** identargv    = new char*[identargc];          // Container to store arguments.
        identargv[0]        = const_cast<char*>("identify"); // String content is not important but must be present.
        identargv[1]        = const_cast<char*>("-format");  // We will pass percent escape options.
        QByteArray ba       = filters.toLatin1();
        identargv[2]        = ba.data();                     // Percent escape format description.
        ba                  = filePath.toLatin1();
        identargv[3]        = ba.data();                     // The file path to parse (even if this also passed through IM::ImageInfo container).

        // Call ImageMagick core identification.
        // This is a fast IM C API call, not the IM CLI tool process.
/*
        // NOTE: to hack with CLI IM tool
        qCDebug(DIGIKAM_METAENGINE_LOG) << "IM identify escape format string (" << filters.size() << "bytes):" << Qt::endl << filters << Qt::endl;
*/
        if (IdentifyImageCommand(image_info, identargc, identargv, metadata, &ex) == MagickTrue)
        {
            // Post process metadata

            registerXmpNameSpace(QLatin1String("https://imagemagick.org/MIFF/"), QLatin1String("MIFF")); // Magick Image File Format (Attributes)
            registerXmpNameSpace(QLatin1String("https://imagemagick.org/MIFP/"), QLatin1String("MIFP")); // Magick Image File Properties

            QString output;
            int lbytes = 0;

            for (int i = 0 ; i < msize ; ++i)
            {
                if (metadata[i])
                {
                    QString tmp = QString::fromUtf8(metadata[i]);
                    output.append(tmp);
                    lbytes     += tmp.size();
                    Q_UNUSED(lbytes);
/*
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Append new metadata line of" << tmp.size() << "bytes";
                    qCDebug(DIGIKAM_METAENGINE_LOG) << metadata[i];
*/
                }
            }

            QStringList lines = output.split(QLatin1Char('\n'));
/*
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Total metadata lines parsed:" << lines.count() << "Total bytes:" << lbytes;
*/
            QString key;
            QString val;

            for (const QString& tuple : EXIV2_AS_CONST(lines))
            {
                key = tuple.section(QLatin1Char('='), 0, 0);
                val = tuple.section(QLatin1Char('='), 1, 1);

                if (val.startsWith(QLatin1Char('\'')))
                {
                    val = val.section(QLatin1Char('\''), 1, 1);             // Drop tag description string on the right side (stage1).
                    val = val.remove(QLatin1Char('\''));
                }

                if (val.contains(QLatin1String(" / ")))
                {
                    val = val.section(QLatin1String(" / "), 0, 0);          // Drop tag description string on the right side (stage2).
                }

                if (
                    key.isEmpty()                                 ||
                    key.startsWith(QLatin1String("comment"))      ||
                    key.startsWith(QLatin1String("date:create"))  ||
                    key.startsWith(QLatin1String("date:modify"))
                   )
                {
                    // These tags are already handled with Exif or key or val are empty.

                    continue;
                }

                if (val.isEmpty())
                {
                    val = QLatin1String("None");        // Mimic IM "none" strings, not i18n
                }

                if (!key.startsWith(QLatin1String("Xmp.")))
                {
                    // Create a dedicated XMP namespace to store ImageMagick properties.

                    key = QLatin1String("Xmp.MIFP.") + key.remove(QLatin1Char(':'));
                }

                key = key.remove(QLatin1Char('-'));
                key = key.remove(QLatin1Char('_'));
                val = val.simplified();

                // Register XMP tags in container

                if      (
                         key == QLatin1String("Xmp.tiff.ImageDescription") ||
                         key == QLatin1String("Xmp.exif.UserComment")
                        )
                {
                    setXmpTagStringLangAlt(key.toLatin1().constData(), val, QString());
                }
                else if (key == QLatin1String("Xmp.tiff.BitsPerSample"))
                {
                    setXmpTagStringSeq(key.toLatin1().constData(), QStringList() << val);
                }
                else
                {
                    setXmpTagString(key.toLatin1().constData(), val);
                }
            }

            QString dt = getXmpTagString("Xmp.tiff.DateTime");

            if (!dt.isEmpty())
            {
                setXmpTagString("Xmp.exif.DateTimeOriginal", dt);
                setXmpTagString("Xmp.xmp.CreateDate",        dt);
            }

            ret = true;
        }
        else
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Cannot parse metadata from ["
                                              << filePath
                                              << "] with ImageMagick identify";
            ret = false;
        }

        // Clear memory

        DestroyImageInfo(image_info);

        for (int i = 0 ; i < msize ; ++i)
        {
            if (metadata[i])
            {
                // Note: IM use malloc(), not new operator. Do not use delete operator here.

                free (metadata[i]);
            }
        }

        delete [] metadata;
        delete [] identargv;
    }
    catch (Exception& error_)
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Cannot read metadata from ["
                                          << filePath
                                          << "] due to ImageMagick exception:"
                                          << error_.what();
        ret = false;
    }

#else  // HAVE_IMAGE_MAGICK

    Q_UNUSED(filePath);

#endif // HAVE_IMAGE_MAGICK

    return ret;
}

} // namespace Digikam
