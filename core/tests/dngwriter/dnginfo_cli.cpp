/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-07
 * Description : a command line tool to extract info from a DNG file
 *
 * SPDX-FileCopyrightText: 2011      by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>

// DNG SDK includes

#include "dng_host.h"
#include "dng_camera_profile.h"
#include "dng_color_space.h"
#include "dng_file_stream.h"
#include "dng_image.h"
#include "dng_image_writer.h"
#include "dng_info.h"
#include "dng_memory_stream.h"
#include "dng_opcodes.h"
#include "dng_opcode_list.h"
#include "dng_parse_utils.h"
#include "dng_string.h"
#include "dng_render.h"
#include "dng_xmp_sdk.h"

// Local includes

#include "digikam_debug.h"

int main(int argc, char** argv)
{
    const int CHUNK = 65536;

    try
    {
        QCoreApplication app(argc, argv);

        bool extractOriginal = false;
        bool extractIfd      = false;

        if (argc == 1)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "\n"
                        "dnginfo - DNG information tool\n"
                        "Usage: %s [options] dngfile\n"
                        "Valid options:\n"
                        "  -extractraw            extract embedded original\n"
                        "  -extractifd            extract IFD images\n"
                    << argv[0];

            return -1;
        }

        qint32 index;

        for (index = 1 ; index < argc && argv[index][0] == '-' ; ++index)
        {
            QString option = QString::fromUtf8(&argv[index][1]);

            if (option == QLatin1String("extractraw"))
            {
                extractOriginal = true;
            }

            if (option == QLatin1String("extractifd"))
            {
                extractIfd = true;
            }
        }

        if (index == argc)
        {
            qCCritical(DIGIKAM_TESTS_LOG) << "*** No file specified\n";
            return 1;
        }

        QFileInfo dngFileInfo(QString::fromUtf8(argv[index]));

        dng_xmp_sdk::InitializeSDK();

        dng_file_stream stream(QFile::encodeName(dngFileInfo.absoluteFilePath()).constData());
        dng_host        host;
        host.SetKeepOriginalFile(true);

        AutoPtr<dng_negative> negative;
        {
            dng_info info;
            info.Parse(host, stream);
            info.PostParse(host);

            if (!info.IsValidDNG())
            {
                return dng_error_bad_format;
            }

            negative.Reset(host.Make_dng_negative());
            negative->Parse(host, stream, info);
            negative->PostParse(host, stream, info);

            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("Model:           %1").arg(QString::fromLatin1(negative->ModelName().Get()));
            dng_rect defaultCropArea = negative->DefaultCropArea();
            dng_rect activeArea = negative->GetLinearizationInfo()->fActiveArea;
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("FinalImageSize:  %1 x %2").arg(negative->DefaultFinalWidth()).arg(negative->DefaultFinalHeight());
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("RawImageSize:    %1 x %2").arg(info.fIFD[info.fMainIndex]->fImageWidth).arg(info.fIFD[info.fMainIndex]->fImageLength);
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("ActiveArea:      %1, %2 : %3 x %4").arg(activeArea.t).arg(activeArea.l).arg(activeArea.W()).arg(activeArea.H());
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("DefaultCropArea: %1, %2 : %3 x %4").arg(defaultCropArea.t).arg(defaultCropArea.l).arg(defaultCropArea.W()).arg(defaultCropArea.H());
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("OriginalData:    %1 bytes").arg(negative->OriginalRawFileDataLength());
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("PrivateData:     %1 bytes").arg(negative->PrivateLength());
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("CameraProfiles:  %1").arg(negative->ProfileCount());

            qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl;

            for (uint32 i = 0 ; i < negative->ProfileCount() ; ++i)
            {
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  Profile: %1").arg(i);
                dng_camera_profile dcp = negative->ProfileByIndex(i);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Name:      %1").arg(QString::fromLatin1(dcp.Name().Get()));
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Copyright: %1").arg(QString::fromLatin1(dcp.Copyright().Get()));
            }

            qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl;

            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("Opcodes(1):      %1").arg(info.fIFD[info.fMainIndex]->fOpcodeList1Count);
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("Opcodes(2):      %1").arg(info.fIFD[info.fMainIndex]->fOpcodeList2Count);
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("Opcodes(3):      %1").arg(info.fIFD[info.fMainIndex]->fOpcodeList3Count);
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("MainImage:       %1").arg(info.fMainIndex);
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("ChainedCount:    %1").arg(info.ChainedIFDCount());

            qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl;

            for (uint32 ifdIdx = 0 ; ifdIdx < info.IFDCount() ; ++ifdIdx)
            {
                dng_ifd* const ifd = info.fIFD[ifdIdx];

                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("IFD: %1").arg(ifdIdx);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  ImageWidth:    %1").arg(ifd->fImageWidth);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  ImageLength:   %1").arg(ifd->fImageLength);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  BitsPerSample:");

                for (uint32 i = 0 ; i < ifd->fSamplesPerPixel ; ++i)
                {
                    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("     %1").arg(ifd->fBitsPerSample[i]);
                }

                qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl;

                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  Compression:               %1").arg(QLatin1String(LookupCompression(ifd->fCompression)));
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  PhotometricInterpretation: %1").arg(QLatin1String(LookupPhotometricInterpretation(ifd->fPhotometricInterpretation)));
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  SamplesPerPixel:           %1").arg(ifd->fSamplesPerPixel);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  PlanarConfiguration:       %1").arg(ifd->fPlanarConfiguration);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  LinearizationTableCount:   %1").arg(ifd->fLinearizationTableCount);
                qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("  LinearizationTableType:    %1").arg(ifd->fLinearizationTableType);

                qCDebug(DIGIKAM_TESTS_LOG) << Qt::endl;

                if (extractIfd)
                {
                    if ((ifd->fPlanarConfiguration == pcInterleaved) &&
                        (ifd->fCompression         == ccJPEG)        &&
                        (ifd->fSamplesPerPixel     == 3)             &&
                        (ifd->fBitsPerSample[0]    == 8)             &&
                        (ifd->fBitsPerSample[1]    == 8)             &&
                        (ifd->fBitsPerSample[2]    == 8)             &&
                        (ifd->TilesAcross()        == 1)             &&
                        (ifd->TilesDown()          == 1))
                    {
                        uint64 tileOffset    = ifd->fTileOffset[0];
                        uint64 tileLength    = ifd->fTileByteCount[0];
                        uint8* const pBuffer = new uint8[tileLength];
                        stream.SetReadPosition(tileOffset);
                        stream.Get(pBuffer, tileLength);

                        QString outfn2 = QString::fromLatin1("%1-ifd%2.jpg")
                            .arg(dngFileInfo.absoluteFilePath())
                            .arg(ifdIdx);

                        dng_file_stream streamOF2(outfn2.toLatin1().constData(), true);
                        streamOF2.Put(pBuffer, tileLength);

                        delete [] pBuffer;

                        qCDebug(DIGIKAM_TESTS_LOG) << "Extracted IFD image as JPEG:" << outfn2;
                    }
                    else
                    {
                        AutoPtr<dng_image> image;

                        image.Reset(host.Make_dng_image(ifd->Bounds(), ifd->fSamplesPerPixel, ifd->PixelType()));
                        ifd->ReadImage(host, stream, *image.Get());

                        QString outfn = QString::fromLatin1("%1-ifd%2.tif")
                            .arg(dngFileInfo.absoluteFilePath())
                            .arg(ifdIdx);

                        dng_file_stream streamOF(outfn.toLatin1().constData(), true);

                        dng_image_writer writer;
                        writer.WriteTIFF(host, streamOF, *image.Get(), (image->Planes() >= 3) ? piRGB : piBlackIsZero, ccUncompressed);

                        qCDebug(DIGIKAM_TESTS_LOG) << "Extracted IFD image as TIF:" << outfn;
                    }
                }
            }

            QString originalFileName(QString::fromUtf8(negative->OriginalRawFileName().Get()));
            quint32 originalDataLength     = negative->OriginalRawFileDataLength();
            const void* originalData       = negative->OriginalRawFileData();

            if (extractOriginal)
            {
                if (originalDataLength > 0)
                {
                    dng_memory_allocator memalloc(gDefaultDNGMemoryAllocator);
                    dng_memory_stream compressedDataStream(memalloc);
                    compressedDataStream.Put(originalData, originalDataLength);
                    compressedDataStream.SetReadPosition(0);
                    compressedDataStream.SetBigEndian(true);
                    quint32 forkLength = compressedDataStream.Get_uint32();
                    quint32 forkBlocks = (uint32)floor((forkLength + 65535.0) / 65536.0);
                    QVector<quint32> offsets;

                    for (quint32 block = 0 ; block <= forkBlocks ; ++block)
                    {
                        quint32 offset = compressedDataStream.Get_uint32();
                        offsets.push_back(offset);
                    }

                    QFile originalFile(dngFileInfo.absolutePath() + QLatin1Char('/') + originalFileName);
                    qCDebug(DIGIKAM_TESTS_LOG) << "extracting embedded original to " << dngFileInfo.fileName();

                    if (!originalFile.open(QIODevice::WriteOnly))
                    {
                        qCDebug(DIGIKAM_TESTS_LOG) << "Cannot open file. Aborted...";
                        return 1;
                    }

                    QDataStream originalDataStream(&originalFile);

                    for (quint32 block = 0 ; block < forkBlocks ; ++block)
                    {
                        QByteArray compressedDataBlock;
                        compressedDataBlock.resize(offsets[block + 1] - offsets[block]);
                        compressedDataStream.Get(compressedDataBlock.data(), compressedDataBlock.size());
                        quint32 uncompressedDataSize = qMin((quint32)CHUNK, forkLength);

                        compressedDataBlock.prepend(uncompressedDataSize         & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >>  8) & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >> 16) & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >> 24) & 0xFF);
                        forkLength -= uncompressedDataSize;

                        QByteArray originalDataBlock = qUncompress(reinterpret_cast<const uchar*>(compressedDataBlock.data()), compressedDataBlock.size());
/*
                        qCDebug(DIGIKAM_TESTS_LOG) << "compressed data block " << compressedDataBlock.size() << " -> " << originalDataBlock.size();
*/
                        originalDataStream.writeRawData(originalDataBlock.data(), originalDataBlock.size());
                    }

                    originalFile.close();
                }
                else
                {
                    qCWarning(DIGIKAM_TESTS_LOG) << "No embedded originals RAW data found";
                }
            }
        }

        dng_xmp_sdk::TerminateSDK();

        return 0;
    }

    catch (const dng_exception& exception)
    {
        int ret = exception.ErrorCode();
        qCCritical(DIGIKAM_TESTS_LOG) << "DNGWriter: DNG SDK exception code (" << ret << ")";

        return (-1);
    }

    catch (...)
    {
        qCCritical(DIGIKAM_TESTS_LOG) << "DNGWriter: DNG SDK exception code unknown";

        return (-1);
    }
}
