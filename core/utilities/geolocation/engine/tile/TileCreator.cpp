/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "TileCreator.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QDir>
#include <QRect>
#include <QSize>
#include <QVector>
#include <QApplication>
#include <QImage>
#include <QPainter>

// Local includes

#include "MarbleGlobal.h"
#include "MarbleDirs.h"
#include "TileLoaderHelper.h"
#include "digikam_debug.h"
#include "actionthreadbase.h"

using namespace Digikam;

namespace Marble
{

class Q_DECL_HIDDEN TileCreatorPrivate
{
public:

    TileCreatorPrivate(TileCreatorSource* source,
                       const QString& dem,
                       const QString& targetDir = QString())
        : m_dem         (dem),
          m_targetDir   (targetDir),
          m_cancelled   (false),
          m_tileFormat  (QString::fromUtf8("jpg")),
          m_resume      (false),
          m_verify      (false),
          m_source      (source)
    {
        if (m_dem == QLatin1String("true"))
        {
            m_tileQuality = 70;
        }

        else
        {
            m_tileQuality = 85;
        }
    }

    ~TileCreatorPrivate()
    {
        delete m_source;
    }

public:

    QString            m_dem;
    QString            m_targetDir;
    bool               m_cancelled      = false;
    QString            m_tileFormat     = QString::fromUtf8("jpg");
    int                m_tileQuality    = 85;
    bool               m_resume         = false;
    bool               m_verify         = false;

    TileCreatorSource* m_source         = nullptr;

private:

    TileCreatorPrivate(const TileCreatorPrivate&)            = delete;
    TileCreatorPrivate& operator=(const TileCreatorPrivate&) = delete;
};

class Q_DECL_HIDDEN TileCreatorSourceImage : public TileCreatorSource
{
public:

    explicit TileCreatorSourceImage(const QString& sourcePath)
        : m_sourceImage (QImage(sourcePath)),
          m_cachedRowNum(-1)
    {
    }

    QSize fullImageSize() const override
    {
        if ((m_sourceImage.size().width() > 21600) || (m_sourceImage.height() > 10800))
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("Install map too large!");

            return QSize();
        }

        return m_sourceImage.size();
    }

    QImage tile(int n, int m, int maxTileLevel) override
    {
        int  mmax         = TileLoaderHelper::levelToColumn(defaultLevelZeroColumns, maxTileLevel);
        int  nmax         = TileLoaderHelper::levelToRow(defaultLevelZeroRows, maxTileLevel);

        int imageHeight   = m_sourceImage.height();
        int imageWidth    = m_sourceImage.width();

        // If the image size of the image source does not match the expected
        // geometry we need to smooth-scale the image in advance to match
        // the required size
        bool needsScaling = (
                             (imageWidth  != (2 * nmax * (int)(c_defaultTileSize)))  ||
                             (imageHeight != (nmax     * (int)(c_defaultTileSize)))
                            );

        if (needsScaling)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Image Size doesn't match 2*n*TILEWIDTH x n*TILEHEIGHT geometry. Scaling ...";
        }

        int stdImageWidth  = 2 * nmax * c_defaultTileSize;

        if (stdImageWidth == 0)
        {
            stdImageWidth = 2 * c_defaultTileSize;
        }

        int stdImageHeight  = nmax * c_defaultTileSize;

        if (stdImageWidth != imageWidth)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileCreator::createTiles() The size of the final image will measure  %1 x %2 pixels")
                                                .arg(stdImageWidth).arg(stdImageHeight);
        }

        QImage row;

        if (m_cachedRowNum == n)
        {
            row = m_rowCache;
        }
        else
        {

            QRect sourceRowRect(0, (int)((qreal)(n * imageHeight) / (qreal)(nmax)),
                                imageWidth, (int)((qreal)(imageHeight) / (qreal)(nmax)));

            row = m_sourceImage.copy(sourceRowRect);

            if (needsScaling)
            {
                // Pick the current row and smooth scale it
                // to make it match the expected size

                QSize destSize(stdImageWidth, c_defaultTileSize);
                row = row.scaled(destSize,
                                 Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
            }

            m_cachedRowNum = n;
            m_rowCache     = row;
        }

        if (row.isNull())
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Read-Error! Null QImage!";

            return QImage();
        }

        QImage  tile = row.copy(m * stdImageWidth / mmax, 0, c_defaultTileSize, c_defaultTileSize);

        return tile;
    }

private:

    QImage m_sourceImage;
    QImage m_rowCache;
    int    m_cachedRowNum = -1;
};

TileCreator::TileCreator(const QString& sourceDir, const QString& installMap,
                         const QString& dem, const QString& targetDir)
    : QThread(nullptr),
      d      (new TileCreatorPrivate(nullptr, dem, targetDir))

{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Prefix: " << sourceDir
                                << "installmap:" << installMap;

    QString sourcePath;

    // If the sourceDir starts with a '/' assume an absolute path.
    // Otherwise assume a relative marble data path

    if (QDir::isAbsolutePath(sourceDir))
    {
        sourcePath = sourceDir + QLatin1Char('/') + installMap;
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Trying absolute path*:" << sourcePath;
    }
    else
    {
        sourcePath = MarbleDirs::path(QLatin1String("maps/") + sourceDir + QLatin1Char('/') + installMap);
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Trying relative path*:"
                                       << QLatin1String("maps/") + sourceDir + QLatin1Char('/') + installMap;
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Creating tiles from*: " << sourcePath;

    d->m_source = new TileCreatorSourceImage(sourcePath);

    if (d->m_targetDir.isNull())
    {
        d->m_targetDir = MarbleDirs::localPath() + QLatin1String("/maps/")
                         + sourcePath.section(QLatin1Char('/'), -3, -2) + QLatin1Char('/');
    }

    setTerminationEnabled(true);
}

TileCreator::TileCreator(TileCreatorSource* source, const QString& dem, const QString& targetDir)
    : QThread(nullptr),
      d      (new TileCreatorPrivate(source, dem, targetDir))
{
    setTerminationEnabled(true);
}

TileCreator::~TileCreator()
{
    delete d;
}

void TileCreator::cancelTileCreation()
{
    d->m_cancelled = true;
}

void TileCreator::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("MarbleTileCreator"));       // To customize thread name

    if (d->m_resume && (d->m_tileFormat == QLatin1String("jpg")) && (d->m_tileQuality != 100))
    {
        qCWarning(DIGIKAM_GEOENGINE_LOG) << "Resuming jpegs is only supported with tileQuality 100";

        return;
    }

    if (!d->m_targetDir.endsWith(QLatin1Char('/')))
    {
        d->m_targetDir += QLatin1Char('/');
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Installing tiles to: " << d->m_targetDir;

    QVector<QRgb> grayScalePalette;

    for (int cnt = 0 ; cnt <= 255 ; ++cnt)
    {
        grayScalePalette.insert(cnt, qRgb(cnt, cnt, cnt));
    }

    QSize fullImageSize = d->m_source->fullImageSize();
    int  imageWidth     = fullImageSize.width();
    int  imageHeight    = fullImageSize.height();

    qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("TileCreator::createTiles() image dimensions %1 x %2")
                                      .arg(imageWidth).arg(imageHeight);

    if ((imageWidth < 1) || (imageHeight < 1))
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("Invalid imagemap!");
        return;
    }

    // Calculating Maximum Tile Level

    float approxMaxTileLevel = std::log(imageWidth / (2.0 * c_defaultTileSize)) / std::log(2.0);
    int  maxTileLevel        = 0;

    if (approxMaxTileLevel == int(approxMaxTileLevel))
    {
        maxTileLevel = static_cast<int>(approxMaxTileLevel);
    }
    else
    {
        maxTileLevel = static_cast<int>(approxMaxTileLevel + 1);
    }

    if (maxTileLevel < 0)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG)
                << QString::fromUtf8("TileCreator::createTiles(): Invalid Maximum Tile Level: %1")
                   .arg(maxTileLevel);
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Maximum Tile Level: " << maxTileLevel;

    if (!QDir(d->m_targetDir).exists())
    {
        (QDir::root()).mkpath(d->m_targetDir);
    }

    // Counting total amount of tiles to be generated for the progressbar
    // to prevent compiler warnings this var should
    // match the type of maxTileLevel

    int  tileLevel      = 0;
    int  totalTileCount = 0;

    while (tileLevel <= maxTileLevel)
    {
        totalTileCount += (TileLoaderHelper::levelToRow(defaultLevelZeroRows, tileLevel)
                           * TileLoaderHelper::levelToColumn(defaultLevelZeroColumns, tileLevel));
        tileLevel++;
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << totalTileCount << " tiles to be created in total.";

    int  mmax = TileLoaderHelper::levelToColumn(defaultLevelZeroColumns, maxTileLevel);
    int  nmax = TileLoaderHelper::levelToRow(defaultLevelZeroRows, maxTileLevel);

    // Loading each row at highest spatial resolution and cropping tiles

    int      percentCompleted  = 0;
    int      createdTilesCount = 0;
    QString  tileName;

    // Creating directory structure for the highest level

    QString  _dirName(d->m_targetDir
                     + QString::fromUtf8("%1").arg(maxTileLevel));

    if (!QDir(_dirName).exists())
    {
        (QDir::root()).mkpath(_dirName);
    }

    for (int n = 0 ; n < nmax ; ++n)
    {
        QString dirName(d->m_targetDir
                        + QString::fromUtf8("%1/%2").arg(maxTileLevel).arg(n, tileDigits, 10, QLatin1Char('0')));

        if (!QDir(dirName).exists())
        {
            (QDir::root()).mkpath(dirName);
        }
    }

    for (int n = 0 ; n < nmax ; ++n)
    {

        for (int m = 0 ; m < mmax ; ++m)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << QString::fromUtf8("** tile") << m << QString::fromUtf8("x") << n;

            if (d->m_cancelled)
            {
                return;
            }

            tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                       .arg(maxTileLevel)
                       .arg(n, tileDigits, 10, QLatin1Char('0'))
                       .arg(m, tileDigits, 10, QLatin1Char('0'))
                       .arg(d->m_tileFormat);

            if (QFile::exists(tileName) && d->m_resume)
            {
                //qCDebug(DIGIKAM_GEOENGINE_LOG) << tileName << "exists already";
            }
            else
            {
                QImage tile = d->m_source->tile(n, m, maxTileLevel);

                if (tile.isNull())
                {
                    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Read-Error! Null QImage!";

                    return;
                }

                if (d->m_dem == QLatin1String("true"))
                {
                    tile = tile.convertToFormat(QImage::Format_Indexed8,
                                                grayScalePalette,
                                                Qt::ThresholdDither);
                }

                bool ok = tile.save(tileName, d->m_tileFormat.toLatin1().data(),
                                              (d->m_tileFormat == QLatin1String("jpg")) ? 100
                                                                                        : d->m_tileQuality);

                if (!ok)
                {
                    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Error while writing Tile: " << tileName;
                }

                qCDebug(DIGIKAM_GEOENGINE_LOG) << tileName << "size" << QFile(tileName).size();

                if (d->m_verify)
                {
                    QImage writtenTile(tileName);
                    Q_ASSERT(writtenTile.size() == tile.size());

                    for (int i = 0 ; i < writtenTile.size().width() ; ++i)
                    {
                        for (int j = 0 ; j < writtenTile.size().height() ; ++j)
                        {
                            if (writtenTile.pixel(i, j) != tile.pixel(i, j))
                            {
                                unsigned int pixel        = tile.pixel(i, j);
                                unsigned int writtenPixel = writtenTile.pixel(i, j);

                                qCWarning(DIGIKAM_GEOENGINE_LOG) << "***** pixel" << i << j << "is off by" << (pixel - writtenPixel) << "pixel" << pixel << "writtenPixel" << writtenPixel;

                                QByteArray baPixel(reinterpret_cast<const char*>(&pixel), sizeof(unsigned int));

                                qCWarning(DIGIKAM_GEOENGINE_LOG) << "pixel" << baPixel.size() << "0x" << baPixel.toHex();

                                QByteArray baWrittenPixel(reinterpret_cast<const char*>(&writtenPixel), sizeof(unsigned int));

                                qCWarning(DIGIKAM_GEOENGINE_LOG) << "writtenPixel" << baWrittenPixel.size() << "0x" << baWrittenPixel.toHex();

                                Q_ASSERT(false);
                            }
                        }
                    }
                }
            }

            percentCompleted = (int)(90 * (qreal)(createdTilesCount)
                                        / (qreal)(totalTileCount));
            createdTilesCount++;

            qCDebug(DIGIKAM_GEOENGINE_LOG) << "percentCompleted" << percentCompleted;

            Q_EMIT progress(percentCompleted);
        }
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "tileLevel: " << maxTileLevel << " successfully created.";

    tileLevel = maxTileLevel;

    // Now that we have the tiles at the highest resolution lets build
    // them together four by four.

    while (tileLevel > 0)
    {
        tileLevel--;

        int nmaxit = TileLoaderHelper::levelToRow(defaultLevelZeroRows, tileLevel);

        for (int n = 0 ; n < nmaxit ; ++n)
        {
            QString  dirName(d->m_targetDir
                             + QString::fromUtf8("%1/%2")
                             .arg(tileLevel)
                             .arg(n, tileDigits, 10, QLatin1Char('0')));

            // qCDebug(DIGIKAM_GEOENGINE_LOG) << "dirName: " << dirName;

            if (!QDir(dirName).exists())
            {
                (QDir::root()).mkpath(dirName);
            }

            int mmaxit = TileLoaderHelper::levelToColumn(defaultLevelZeroColumns, tileLevel);

            for (int m = 0 ; m < mmaxit ; ++m)
            {
                if (d->m_cancelled)
                {
                    return;
                }

                QString newTileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                                      .arg(tileLevel)
                                      .arg(n, tileDigits, 10, QLatin1Char('0'))
                                      .arg(m, tileDigits, 10, QLatin1Char('0'))
                                      .arg(d->m_tileFormat);

                if (QFile::exists(newTileName) && d->m_resume)
                {
                    //qCDebug(DIGIKAM_GEOENGINE_LOG) << newTileName << "exists already";
                }
                else
                {
                    tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                               .arg(tileLevel + 1)
                               .arg(2 * n, tileDigits, 10, QLatin1Char('0'))
                               .arg(2 * m, tileDigits, 10, QLatin1Char('0'))
                               .arg(d->m_tileFormat);
                    QImage  img_topleft(tileName);

                    tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                               .arg(tileLevel + 1)
                               .arg(2 * n, tileDigits, 10, QLatin1Char('0'))
                               .arg(2 * m + 1, tileDigits, 10, QLatin1Char('0'))
                               .arg(d->m_tileFormat);
                    QImage  img_topright(tileName);

                    tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                               .arg(tileLevel + 1)
                               .arg(2 * n + 1, tileDigits, 10, QLatin1Char('0'))
                               .arg(2 * m, tileDigits, 10, QLatin1Char('0'))
                               .arg(d->m_tileFormat);
                    QImage  img_bottomleft(tileName);

                    tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                               .arg(tileLevel + 1)
                               .arg(2 * n + 1, tileDigits, 10, QLatin1Char('0'))
                               .arg(2 * m + 1, tileDigits, 10, QLatin1Char('0'))
                               .arg(d->m_tileFormat);
                    QImage  img_bottomright(tileName);

                    QSize const expectedSize(c_defaultTileSize, c_defaultTileSize);

                    if (
                        (img_topleft.size()     != expectedSize) ||
                        (img_topright.size()    != expectedSize) ||
                        (img_bottomleft.size()  != expectedSize) ||
                        (img_bottomright.size() != expectedSize)
                       )
                    {
                        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Tile write failure. Missing write permissions?";

                        Q_EMIT progress(100);

                        return;
                    }

                    QImage tile = img_topleft;

                    if (d->m_dem == QLatin1String("true"))
                    {
                        tile.setColorTable(grayScalePalette);
                        uchar* destLine = nullptr;

                        for (uint y = 0 ; y < c_defaultTileSize / 2 ; ++y)
                        {
                            destLine             = tile.scanLine(y);
                            const uchar* srcLine = img_topleft.scanLine(2 * y);

                            for (uint x = 0 ; x < c_defaultTileSize / 2 ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * x ];
                            }
                        }

                        for (uint y = 0 ; y < c_defaultTileSize / 2 ; ++y)
                        {
                            destLine             = tile.scanLine(y);
                            const uchar* srcLine = img_topright.scanLine(2 * y);

                            for (uint x = c_defaultTileSize / 2 ; x < c_defaultTileSize ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * (x - c_defaultTileSize / 2) ];
                            }
                        }

                        for (uint y = c_defaultTileSize / 2 ; y < c_defaultTileSize ; ++y)
                        {
                            destLine             = tile.scanLine(y);
                            const uchar* srcLine = img_bottomleft.scanLine(2 * (y - c_defaultTileSize / 2));

                            for (uint x = 0; x < c_defaultTileSize / 2; ++x)
                            {
                                destLine[ x ] = srcLine[ 2 * x ];
                            }
                        }

                        for (uint y = c_defaultTileSize / 2 ; y < c_defaultTileSize ; ++y)
                        {
                            destLine             = tile.scanLine(y);
                            const uchar* srcLine = img_bottomright.scanLine(2 * (y - c_defaultTileSize / 2));

                            for (uint x = c_defaultTileSize / 2 ; x < c_defaultTileSize ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * (x - c_defaultTileSize / 2) ];
                            }
                        }
                    }
                    else
                    {
                        // tile.depth() != 8

                        img_topleft     = img_topleft.convertToFormat(QImage::Format_ARGB32);
                        img_topright    = img_topright.convertToFormat(QImage::Format_ARGB32);
                        img_bottomleft  = img_bottomleft.convertToFormat(QImage::Format_ARGB32);
                        img_bottomright = img_bottomright.convertToFormat(QImage::Format_ARGB32);
                        tile            = img_topleft;

                        QRgb* destLine = nullptr;

                        for (uint y = 0 ; y < c_defaultTileSize / 2 ; ++y)
                        {
                            destLine            = reinterpret_cast<QRgb*>(tile.scanLine(y));
                            const QRgb* srcLine = reinterpret_cast<QRgb*>(img_topleft.scanLine(2 * y));

                            for (uint x = 0 ; x < c_defaultTileSize / 2 ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * x ];
                            }
                        }

                        for (uint y = 0 ; y < c_defaultTileSize / 2 ; ++y)
                        {
                            destLine            = reinterpret_cast<QRgb*>(tile.scanLine(y));
                            const QRgb* srcLine = reinterpret_cast<QRgb*>(img_topright.scanLine(2 * y));

                            for (uint x = c_defaultTileSize / 2 ; x < c_defaultTileSize ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * (x - c_defaultTileSize / 2) ];
                            }
                        }

                        for (uint y = c_defaultTileSize / 2 ; y < c_defaultTileSize ; ++y)
                        {
                            destLine            = reinterpret_cast<QRgb*>(tile.scanLine(y));
                            const QRgb* srcLine = reinterpret_cast<QRgb*>(img_bottomleft.scanLine(2 * (y - c_defaultTileSize / 2)));

                            for (uint x = 0 ; x < c_defaultTileSize / 2 ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * x ];
                            }
                        }

                        for (uint y = c_defaultTileSize / 2 ; y < c_defaultTileSize ; ++y)
                        {
                            destLine            = reinterpret_cast<QRgb*>(tile.scanLine(y));
                            const QRgb* srcLine = reinterpret_cast<QRgb*>(img_bottomright.scanLine(2 * (y - c_defaultTileSize / 2)));

                            for (uint x = c_defaultTileSize / 2 ; x < c_defaultTileSize ; ++x)
                            {
                                destLine[x] = srcLine[ 2 * (x - c_defaultTileSize / 2) ];
                            }
                        }
                    }

                    qCDebug(DIGIKAM_GEOENGINE_LOG) << newTileName;

                    // Saving at 100% JPEG quality to have a high-quality
                    // version to create the remaining needed tiles from.

                    bool  ok = tile.save(newTileName,
                                         d->m_tileFormat.toLatin1().data(),
                                         (d->m_tileFormat == QLatin1String("jpg")) ? 100 : d->m_tileQuality);

                    if (! ok)
                    {
                        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Error while writing Tile: " << newTileName;
                    }
                }

                percentCompleted = (int)(90 * (qreal)(createdTilesCount)
                                         / (qreal)(totalTileCount));
                createdTilesCount++;

                Q_EMIT progress(percentCompleted);

                qCDebug(DIGIKAM_GEOENGINE_LOG) << "percentCompleted" << percentCompleted;
            }
        }

        qCDebug(DIGIKAM_GEOENGINE_LOG) << "tileLevel: " << tileLevel << " successfully created.";
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Tile creation completed.";

    if ((d->m_tileFormat == QLatin1String("jpg")) && (d->m_tileQuality != 100))
    {
        // Applying correct lower JPEG compression now that we created all tiles

        int savedTilesCount = 0;
        tileLevel           = 0;

        while (tileLevel <= maxTileLevel)
        {
            int nmaxit = TileLoaderHelper::levelToRow(defaultLevelZeroRows, tileLevel);

            for (int n = 0 ; n < nmaxit ; ++n)
            {
                int mmaxit = TileLoaderHelper::levelToColumn(defaultLevelZeroColumns, tileLevel);

                for (int m = 0 ; m < mmaxit ; ++m)
                {
                    if (d->m_cancelled)
                    {
                        return;
                    }

                    savedTilesCount++;

                    tileName = d->m_targetDir + QString::fromUtf8("%1/%2/%2_%3.%4")
                               .arg(tileLevel)
                               .arg(n, tileDigits, 10, QLatin1Char('0'))
                               .arg(m, tileDigits, 10, QLatin1Char('0'))
                               .arg(d->m_tileFormat);
                    QImage tile(tileName);

                    bool ok = tile.save(tileName, d->m_tileFormat.toLatin1().data(), d->m_tileQuality);

                    if (!ok)
                    {
                        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Error while writing Tile: " << tileName;
                    }

                    // Don't exceed 99% as this would cancel the thread unexpectedly

                    percentCompleted = 90 + (int)(9 * (qreal)(savedTilesCount)
                                                  / (qreal)(totalTileCount));
                    Q_EMIT progress(percentCompleted);

                    qCDebug(DIGIKAM_GEOENGINE_LOG) << "percentCompleted" << percentCompleted;

                    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "Saving Tile #" << savedTilesCount
                    //         << " of " << totalTileCount
                    //         << " Percent: " << percentCompleted;
                }
            }

            tileLevel++;
        }
    }

    percentCompleted = 100;

    Q_EMIT progress(percentCompleted);

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "percentCompleted: " << percentCompleted;
}

void TileCreator::setTileFormat(const QString& format)
{
    d->m_tileFormat = format;
}

QString TileCreator::tileFormat() const
{
    return d->m_tileFormat;
}

void TileCreator::setTileQuality(int quality)
{
    d->m_tileQuality = quality;
}

int TileCreator::tileQuality() const
{
    return d->m_tileQuality;
}

void TileCreator::setResume(bool _resume)
{
    d->m_resume = _resume;
}

bool TileCreator::resume() const
{
    return d->m_resume;
}

void TileCreator::setVerifyExactResult(bool verify)
{
    d->m_verify = verify;
}

bool TileCreator::verifyExactResult() const
{
    return d->m_verify;
}

} // namespace Marble

#include "moc_TileCreator.cpp"
