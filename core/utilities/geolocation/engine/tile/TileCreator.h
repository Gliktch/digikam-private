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

#pragma once

// Qt includes

#include <QString>
#include <QThread>

// Local includes

#include "digikam_export.h"

class QSize;
class QImage;

namespace Marble
{

class TileCreatorPrivate;

/**
 * Base Class for custom tile source
 *
 * Implement this class to have more control over the tile creating process. This
 * is needed when creating with a high tileLevel where the whole source image cannot
 * be loaded at once.
 **/
class DIGIKAM_EXPORT TileCreatorSource
{
public:

    virtual ~TileCreatorSource() {}

    /**
     * Must return the full size of the source image
     */
    virtual QSize fullImageSize()                       const = 0;

    /**
     * Must return one specific tile
     *
     * tileLevel can be used to calculate the number of tiles in a row or column
     */
    virtual QImage tile(int n, int m, int tileLevel)          = 0;
};

class DIGIKAM_EXPORT TileCreator : public QThread
{
    Q_OBJECT

public:

    /**
     * Constructor for standard Image source
     */
    TileCreator(const QString& sourceDir,
                const QString& installMap,
                const QString& dem,
                const QString& targetDir = QString());

    /**
     * Constructor for own, custom source class
     *
     * Ownership of source is taken by TileCreator
     */
    TileCreator(TileCreatorSource* source, const QString& dem, const QString& targetDir);

    ~TileCreator()                                    override;

    void cancelTileCreation();

    void setTileFormat(const QString& format);
    void setTileQuality(int quality);
    void setResume(bool _resume);
    void setVerifyExactResult(bool verify);
    QString tileFormat()                        const;
    int tileQuality()                           const;
    bool resume()                               const;
    bool verifyExactResult()                    const;

protected:

    void run()                                        override;

Q_SIGNALS:

    void  progress(int value);

private:

    Q_DISABLE_COPY(TileCreator)

private:

    TileCreatorPrivate* const d = nullptr;
};

} // namespace Marble
