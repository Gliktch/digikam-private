/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Export to target DNG.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

// Local includes

#include "dngwriterhost.h"

namespace Digikam
{

int DNGWriter::Private::exportTarget(DNGWriterHost& host,
                                     AutoPtr<dng_negative>& negative,
                                     AutoPtr<dng_image>& image)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Build DNG Negative";

    // Assign Raw image data.

    negative->SetStage1Image(image);

    // Compute linearized and range mapped image

    negative->BuildStage2Image(host);

    // Compute demosaiced image (used by preview and thumbnail)

    negative->SetStage3Gain(0.6);
    negative->BuildStage3Image(host);

    negative->SynchronizeMetadata();
    negative->RebuildIPTC(true);

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    // -----------------------------------------------------------------------------------------

    dng_preview_list previewList;

    for (int previewIndex = 0 ; previewIndex < 2 ; ++previewIndex)
    {
        if      ((previewIndex == 1) && (previewMode == DNGWriter::NONE))
        {
            break;
        }
        else if (previewIndex == 0)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG thumbnail creation";
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG preview image creation";
        }

        if (cancel)
        {
            return PROCESS_CANCELED;
        }

        AutoPtr<dng_image> previewImage;
        {
            dng_render render(host, *negative.Get());
            render.SetFinalSpace(negative->IsMonochrome() ? dng_space_GrayGamma22::Get()
                                                          : dng_space_sRGB::Get());

            render.SetFinalPixelType (ttByte);
            render.SetMaximumSize((previewIndex == 0) ? 256
                                                      : (previewMode == MEDIUM) ? 1280
                                                                                : width);
            previewImage.Reset(render.Render());
        }

        bool useCompressedPreview = (
                                     (negative->RawLossyCompressedImage() != nullptr) ||
                                     (previewIndex > 0)
                                    );

        AutoPtr<dng_preview> preview(useCompressedPreview ? reinterpret_cast<dng_preview*>(new dng_jpeg_preview)
                                                          : reinterpret_cast<dng_preview*>(new dng_image_preview));

        preview->fInfo.fApplicationName.Set_ASCII(QString::fromLatin1("digiKam").toLatin1().constData());
        preview->fInfo.fApplicationVersion.Set_ASCII(digiKamVersion().toLatin1().constData());
        preview->fInfo.fDateTime   = orgDateTimeInfo.Encode_ISO_8601();
        preview->fInfo.fColorSpace = (previewImage->Planes() == 1) ? previewColorSpace_GrayGamma22
                                                                   : previewColorSpace_sRGB;

        if (!useCompressedPreview)
        {
            dng_image_preview* const imagePreview = dynamic_cast<dng_image_preview*>(preview.Get());

            if (!imagePreview)
            {
                continue;
            }

            imagePreview->SetImage(host, previewImage.Release());
        }
        else
        {
            dng_jpeg_preview* const jpegPreview = dynamic_cast<dng_jpeg_preview*>(preview.Get());

            if (!jpegPreview)
            {
                continue;
            }

            int quality = (previewIndex == 0) ? 8 : 5;
            dng_image_writer writer;
            writer.EncodeJPEGPreview(host, *previewImage, *jpegPreview, quality);
        }

        previewList.Append(preview);
    }

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    // -----------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Creating DNG file" << outputInfo.fileName();

    dng_file_stream filestream(dngFilePath.toUtf8().constData(), true);

    dng_image_writer writer;

    writer.WriteDNG(host,
                    filestream,
                    *negative.Get(),
                    &previewList,
                    dngVersion_SaveDefault,
                    !jpegLossLessCompression
                   );

    // -----------------------------------------------------------------------------------------
    // Metadata makernote cleanup using Exiv2 for some RAW file types
    // See bug #204437 and #210371, and write XMP Sidecar if necessary
    // We disable writing to RAW files, see bug #381432
/*
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (meta->load(dngFilePath))
    {
        if (inputInfo.suffix().toUpper() == QLatin1String("ORF"))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: cleanup makernotes using Exiv2";

            meta->setWriteDngFiles(true);
            meta->removeExifTag("Exif.OlympusIp.BlackLevel");
        }
        else
        {
            // Don't touch DNG file and create XMP sidecar depending of host application settings.
            meta->setWriteDngFiles(false);
        }

        meta->applyChanges();
    }
*/
    // -----------------------------------------------------------------------------------------
    // update modification time if desired

    if (updateFileDate)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Setting modification date from meta data:" << fileDate.toString();

        DFileOperations::setModificationTime(dngFilePath, fileDate);
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
