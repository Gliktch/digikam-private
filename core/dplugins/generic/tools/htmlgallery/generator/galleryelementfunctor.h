/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "gallerynamehelper.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryInfo;
class GalleryGenerator;
class GalleryElement;

/**
 * This functor generates images (full and thumbnail) for an url and returns an
 * GalleryElement initialized to fill the xml writer.
 * It is used as an argument to QtConcurrent::mapped().
 */
class GalleryElementFunctor
{
public:

    typedef GalleryElement result_type;

public:

    explicit GalleryElementFunctor(GalleryGenerator* const generator,
                                   GalleryInfo* const info,
                                   const QString& destDir);
    ~GalleryElementFunctor() = default;

    void operator()(GalleryElement& element);

private:

    void   emitWarning(const QString& msg);
    QImage generateThumbnail(const QImage& fullImage, int size, bool square) const;

private:

    // NOTE: Do not use a d private internal container here.

    GalleryGenerator* m_generator = nullptr;
    GalleryInfo*      m_info      = nullptr;
    QString           m_destDir;
    GalleryNameHelper m_uniqueNameHelper;
};

} // namespace DigikamGenericHtmlGalleryPlugin
