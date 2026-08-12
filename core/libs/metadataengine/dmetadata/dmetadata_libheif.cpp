/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-26
 * Description : item metadata interface - libheif helpers.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <qplatformdefs.h>

// Libheif includes

#include <libheif/heif.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

static int64_t heifQIODeviceMetaGetPosition(void* userdata)      // krazy:exclude=typedefs
{
    QFile* const file = static_cast<QFile*>(userdata);

    return (int64_t)file->pos();
}

static int heifQIODeviceMetaRead(void* data, size_t size, void* userdata)
{
    QFile* const file = static_cast<QFile*>(userdata);

    if ((file->pos() + (qint64)size) > file->size())
    {
        return 0;
    }

    qint64 bytes = file->read(reinterpret_cast<char*>(data), size);

    return (int)((file->error() != QFileDevice::NoError) || bytes != (qint64)size);
}

static int heifQIODeviceMetaSeek(int64_t position, void* userdata)  // krazy:exclude=typedefs
{
    QFile* const file = static_cast<QFile*>(userdata);

    return (int)file->seek(position);
}

static heif_reader_grow_status heifQIODeviceMetaWait(int64_t target_size, void* userdata)   // krazy:exclude=typedefs
{
    const QFile* const file = static_cast<QFile*>(userdata);

    if ((qint64)target_size > file->size())
    {
        return heif_reader_grow_status_size_beyond_eof;
    }

    return heif_reader_grow_status_size_reached;
}

bool s_isHeifSuccess(const struct heif_error* const error)
{
    if (error->code == 0)
    {
        return true;
    }

    qCWarning(DIGIKAM_METAENGINE_LOG) << "Error while processing HEIF image:" << error->message;

    return false;
}

void s_readHEICMetadata(struct heif_context* const heif_context, heif_item_id image_id,
                        QByteArray& exif, QByteArray& iptc, QByteArray& xmp)
{
    struct heif_image_handle* image_handle = nullptr;
    struct heif_error error1               = heif_context_get_image_handle(heif_context,
                                                                           image_id,
                                                                           &image_handle);

    if (!s_isHeifSuccess(&error1))
    {
        heif_image_handle_release(image_handle);

        return;
    }

    heif_item_id dataIds[10];

    int count = heif_image_handle_get_list_of_metadata_block_IDs(image_handle,
                                                                 nullptr,
                                                                 dataIds,
                                                                 10);

    qDebug(DIGIKAM_METAENGINE_LOG) << "Found" << count << "HEIF metadata chunk";

    if (count > 0)
    {
        for (int i = 0 ; i < count ; ++i)
        {
            qDebug(DIGIKAM_METAENGINE_LOG) << "Parsing HEIF metadata chunk:" << heif_image_handle_get_metadata_type(image_handle, dataIds[i]);

            if (QLatin1String(heif_image_handle_get_metadata_type(image_handle, dataIds[i])) == QLatin1String("Exif"))
            {
                // Read Exif chunk.

                size_t length = heif_image_handle_get_metadata_size(image_handle, dataIds[i]);

                QByteArray exifChunk;
                exifChunk.resize((int)length);

                struct heif_error error2 = heif_image_handle_get_metadata(image_handle,
                                                                          dataIds[i],
                                                                          exifChunk.data());

                if ((error2.code == 0) && (length > 4))
                {
                    // The first 4 bytes indicate the
                    // offset to the start of the TIFF header of the Exif data.

                    int skip = (
                                (exifChunk.constData()[0] << 24) |
                                (exifChunk.constData()[1] << 16) |
                                (exifChunk.constData()[2] << 8)  |
                                 exifChunk.constData()[3]
                               ) + 4;

                    if (exifChunk.size() > skip)
                    {
                        // Copy the real exif data into the byte array

                        qDebug(DIGIKAM_METAENGINE_LOG) << "HEIF exif container found with size:" << length - skip;
                        exif.append(reinterpret_cast<char*>(exifChunk.data() + skip), exifChunk.size() - skip);
                    }
                }
            }

            if (QLatin1String(heif_image_handle_get_metadata_type(image_handle, dataIds[i])) == QLatin1String("iptc"))
            {
                // Read Iptc chunk.

                size_t length = heif_image_handle_get_metadata_size(image_handle, dataIds[i]);

                iptc.resize((int)length);

                struct heif_error error3 = heif_image_handle_get_metadata(image_handle,
                                                                          dataIds[i],
                                                                          iptc.data());

                if (error3.code == 0)
                {
                    qDebug(DIGIKAM_METAENGINE_LOG) << "HEIF iptc container found with size:" << length;
                }
                else
                {
                    iptc = QByteArray();
                }
            }

            if (
                (QLatin1String(heif_image_handle_get_metadata_type(image_handle, dataIds[i]))         == QLatin1String("mime")) &&
                (QLatin1String(heif_image_handle_get_metadata_content_type(image_handle, dataIds[i])) == QLatin1String("application/rdf+xml"))
               )
            {
                // Read Xmp chunk.

                size_t length = heif_image_handle_get_metadata_size(image_handle, dataIds[i]);

                xmp.resize((int)length);

                struct heif_error error4 = heif_image_handle_get_metadata(image_handle,
                                                                          dataIds[i],
                                                                          xmp.data());

                if (error4.code == 0)
                {
                    qDebug(DIGIKAM_METAENGINE_LOG) << "HEIF xmp container found with size:" << length;
                }
                else
                {
                    xmp = QByteArray();
                }
            }
        }
    }

    heif_image_handle_release(image_handle);
}

bool DMetadata::loadUsingLibheif(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toUpper();

    if (
        !fileInfo.exists() || ext.isEmpty() ||
        (ext != QLatin1String("HEIF"))      ||
        (ext != QLatin1String("HEIC"))      ||
        (ext != QLatin1String("HIF"))
       )
    {
        return false;
    }

    QFile readFile(filePath);

    if (!readFile.open(QIODevice::ReadOnly))
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Error: Could not open source file.";

        return false;
    }

    const qint64 headerLen = 12;

    QByteArray header(headerLen, '\0');

    if (readFile.read(reinterpret_cast<char*>(header.data()), headerLen) != headerLen)
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Error: Could not parse magic identifier.";

        return false;
    }

    if (
        (memcmp(&header.data()[4], "ftyp", 4) != 0) &&
        (memcmp(&header.data()[8], "heic", 4) != 0) &&
        (memcmp(&header.data()[8], "heix", 4) != 0) &&
        (memcmp(&header.data()[8], "mif1", 4) != 0)
       )
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Error: source file is not HEIF image.";

        return false;
    }

    readFile.reset();

    // -------------------------------------------------------------------
    // Initialize HEIF API.

    heif_item_id primary_image_id;

    struct heif_context* const heif_context = heif_context_alloc();

    heif_reader reader;
    reader.reader_api_version = 1;
    reader.get_position       = heifQIODeviceMetaGetPosition;
    reader.read               = heifQIODeviceMetaRead;
    reader.seek               = heifQIODeviceMetaSeek;
    reader.wait_for_file_size = heifQIODeviceMetaWait;

    struct heif_error error   = heif_context_read_from_reader(heif_context,
                                                              &reader,
                                                              reinterpret_cast<void*>(&readFile),
                                                              nullptr);

    if (!s_isHeifSuccess(&error))
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Error: Could not read source file.";
        heif_context_free(heif_context);

        return false;
    }

    error = heif_context_get_primary_image_ID(heif_context, &primary_image_id);

    if (!s_isHeifSuccess(&error))
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "Error: Could not load image data.";
        heif_context_free(heif_context);

        return false;
    }

    QByteArray exif;
    QByteArray iptc;
    QByteArray xmp;
    bool ret = false;

    s_readHEICMetadata(heif_context, primary_image_id, exif, iptc, xmp);

    if (!exif.isEmpty() || !iptc.isEmpty() || !xmp.isEmpty())
    {
        if (!exif.isEmpty())
        {
            setExif(exif);
        }

        if (!iptc.isEmpty())
        {
            setIptc(iptc);
        }

        if (!xmp.isEmpty())
        {
            setXmp(xmp);
        }

        ret = true;
    }

    heif_context_free(heif_context);

    return ret;
}

} // namespace Digikam
