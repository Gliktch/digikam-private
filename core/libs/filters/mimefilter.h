/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by type mime
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QComboBox>

namespace Digikam
{

class MimeFilter : public QComboBox
{
    Q_OBJECT

public:

    enum TypeMimeFilter
    {
        AllFiles = 0,
        ImageFiles,
        RAWFiles,            ///< All Raw file formats such as nef, cr2, arw, pef, etc..
        DNGFiles,
        NoRAWFiles,
        JPGFiles,
        JPEG2000Files,
        JPEGXLFiles,
        WEBPFiles,
        PNGFiles,
        TIFFiles,
        PGFFiles,
        HEIFFiles,           ///< HEVC H265 compression based containers.
        AVIFFiles,
        MoviesFiles,
        AudioFiles,
        RasterGraphics       ///< PSD, XCF, etc...
    };

public:

    explicit MimeFilter(QWidget* const parent);
    ~MimeFilter() override = default;

    void setMimeFilter(int filter);
    int mimeFilter();
};

} // namespace Digikam
