/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QVariant>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "focuspoint.h"
#include "metaengine.h"

namespace Digikam
{

class DIGIKAM_EXPORT FocusPointsExtractor : public QObject
{
    Q_OBJECT

public:

    /**
     * A list used to store focus points of a image extracted from meta data
     *
     * With extract() function, an exiftool parser is used to read data from
     * metadata and lists all focus points. Each focus point is defined by their
     * relative centers coordinate and relative size. Each point has own
     * type (Inactive, Infocus, Selected, SelectedInFocus)
     */
    using ListAFPoints = QList<FocusPoint>;

public:

    explicit FocusPointsExtractor(QObject* const parent, const QString& path);
    ~FocusPointsExtractor();

public:

    ListAFPoints get_af_points();
    ListAFPoints get_af_points(FocusPoint::TypePoint type);

    bool    isAFPointsReadOnly()                                                                            const;
    QString make()                                                                                          const;
    QString model()                                                                                         const;
    QSize   originalSize()                                                                                  const;
    MetaEngine::ImageOrientation orientation()                                                              const;

private:

    QVariant findValue(const QString& tagName, bool isList = false)                                         const;
    QVariant findValue(const QString& tagNameRoot, const QString& key, bool isList = false)                 const;
    QVariant findValueFirstMatch(const QStringList& listTagNames, bool isList = false)                      const;
    QVariant findValueFirstMatch(const QString& tagNameRoot, const QStringList& keys, bool isList = false)  const;
    QVariant findNumValue(const QString& tagName)                                                           const;

    void setAFPointsReadOnly(bool readOnly)                                                                 const;
    void setOriginalSize(const QSize& size)                                                                 const;

    ListAFPoints findAFPoints()                                                                             const;

    ListAFPoints getAFPoints_canon()                                                                        const;
    ListAFPoints getAFPoints_exif()                                                                         const;
    ListAFPoints getAFPoints_fujifilm()                                                                     const;
    ListAFPoints getAFPoints_nikon()                                                                        const;
    ListAFPoints getAFPoints_olympus()                                                                      const;
    ListAFPoints getAFPoints_panasonic()                                                                    const;
    ListAFPoints getAFPoints_sony()                                                                         const;
    ListAFPoints getAFPoints_xmp()                                                                          const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
