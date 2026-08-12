/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-15
 * Description : digiKam 8/16 bits image management API.
 *               Private data container.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// C ANSI includes

#ifndef Q_OS_WIN
extern "C"
{
#endif

#include <stdint.h>

#ifndef Q_OS_WIN
#include <math.h>
}
#endif

// C++ includes

#include <cstdio>

// Qt includes

#include <QCryptographicHash>
#include <QSharedData>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QMap>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QPaintEngine>
#include <QPainter>
#include <QPixmap>
#include <QSysInfo>
#include <QUuid>
#include <QScopedPointer>
#include <QColorSpace>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "dplugindimg.h"
#include "digikam_export.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "dimagehistory.h"
#include "iccprofile.h"
#include "metaengine_rotation.h"
#include "drawdecoder.h"
#include "loadsavethread.h"
#include "filereadwritelock.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "dimgloaderobserver.h"
#include "randomnumbergenerator.h"

typedef uint64_t ullong;    // krazy:exclude=typedefs
typedef int64_t  llong;     // krazy:exclude=typedefs

namespace Digikam
{

/**
 * @brief Lanczos kernel is precomputed in a table with this resolution
 * The value below seems to be enough for HQ upscaling up to eight times
 */
static const int LANCZOS_TABLE_RES = 256;

/**
 * @brief A support of 3 gives an overall sharper looking image, but
 * it is a) slower b) gives more sharpening artifacts
 */
static const int LANCZOS_SUPPORT   = 2;

/**
 * @brief Define this to use a floating-point implementation of Lanczos interpolation.
 * The integer implementation is a little bit less accurate, but MUCH faster
 * (even on machines with FPU - ~2.5 times faster on Core2); besides, it will
 * run a hell lot faster on computers without a FPU (e.g. PDAs).
 */
//#define LANCZOS_DATA_FLOAT

#ifdef LANCZOS_DATA_FLOAT
#   define LANCZOS_DATA_TYPE float
    static const float LANCZOS_DATA_ONE = 1.0F;
#else
#   define LANCZOS_DATA_TYPE int
    static const int LANCZOS_DATA_ONE   = 4096;
#endif

class DIGIKAM_EXPORT DImg::Private : public QSharedData
{
public:

    Private() = default;

    ~Private()
    {
        delete [] data;
        delete [] lanczos_func;
    }

public:

    bool                    null            = true;
    bool                    alpha           = false;
    bool                    sixteenBit      = false;

    unsigned int            width           = 0;
    unsigned int            height          = 0;

    unsigned char*          data            = nullptr;
    LANCZOS_DATA_TYPE*      lanczos_func    = nullptr;

    MetaEngineData          metaData;
    QMap<QString, QVariant> attributes;
    QMap<QString, QString>  embeddedText;
    IccProfile              iccProfile;
    DImageHistory           imageHistory;
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT DImgStaticPriv
{
public:

    static DPluginDImg* pluginForFile(const QFileInfo& fileInfo, bool magic)
    {
        QMultiMap<int, DPluginDImg*> pluginMap;
        const auto plugins = DPluginLoader::instance()->allPlugins();

        for (DPlugin* const p : plugins)
        {
            int prio;
            DPluginDImg* const plug = dynamic_cast<DPluginDImg*>(p);

            if (plug && ((prio = plug->canRead(fileInfo, magic)) > 0))
            {
 /*
                qCDebug(DIGIKAM_DIMG_LOG) << "File path:" << filePath
                                          << "Priority:"  << prio
                                          << "Loader:"    << plug->loaderName();
 */
                pluginMap.insert(prio, plug);
            }
        }

        if (pluginMap.isEmpty())
        {
            return nullptr;
        }

        return pluginMap.first();
    }

    static DPluginDImg* pluginForFormat(const QString& format)
    {
        QMultiMap<int, DPluginDImg*> pluginMap;

        if (!format.isNull())
        {
            const auto plugins = DPluginLoader::instance()->allPlugins();

            for (DPlugin* const p : plugins)
            {
                int prio;
                DPluginDImg* const plug = dynamic_cast<DPluginDImg*>(p);

                if (plug && ((prio = plug->canWrite(format)) > 0))
                {
                    pluginMap.insert(prio, plug);
                }
            }
        }

        if (pluginMap.isEmpty())
        {
            return nullptr;
        }

        return pluginMap.first();
    }

    static DImg::FORMAT loaderNameToFormat(const QString& name)
    {
        if      (name.isNull())
        {
            return DImg::NONE;
        }
        else if (name == QLatin1String("JPEG"))
        {
            return DImg::JPEG;
        }
        else if (name == QLatin1String("PNG"))
        {
            return DImg::PNG;
        }
        else if (name == QLatin1String("TIFF"))
        {
            return DImg::TIFF;
        }
        else if (name == QLatin1String("RAW"))
        {
            return DImg::RAW;
        }
        else if (name == QLatin1String("JPEG2000"))
        {
            return DImg::JP2K;
        }
        else if (name == QLatin1String("PGF"))
        {
            return DImg::PGF;
        }
        else if (name == QLatin1String("HEIF"))
        {
            return DImg::HEIF;
        }

        // In others cases, ImageMagick or QImage will be used to try to open file.

        return DImg::QIMAGE;
    }

    static QStringList fileOriginAttributes()
    {
        QStringList list;
        list << QLatin1String("format")
             << QLatin1String("isReadOnly")
             << QLatin1String("originalFilePath")
             << QLatin1String("originalSize")
             << QLatin1String("originalImageHistory")
             << QLatin1String("rawDecodingSettings")
             << QLatin1String("rawDecodingFilterAction")
             << QLatin1String("uniqueHash")
             << QLatin1String("uniqueHashV2")
             << QLatin1String("uniqueHashV3");

        return list;
    }
};

} // namespace Digikam
