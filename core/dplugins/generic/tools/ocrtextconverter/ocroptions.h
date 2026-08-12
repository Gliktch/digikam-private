/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : OCR Tesseract options
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QString>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class OcrOptions
{

public:

    enum class LanguageModes
    {
        /**
         * In the Default mode of Language settings for digital text with multiple languages,
         * Tesseract can automatically recognize languages using Latin alphabets such as English or French,
         * but is not compatible with languages using hieroglyphs such as Chinese, Japanese.
         */
        DEFAULT = 0,

        /**
         * Orientation and Script Detection mode replace Default mode for hieroglyphs languages.
         * See comment on https://invent.kde.org/graphics/digikam/-/merge_requests/177#note_522008
         */
        OSD
    };

    enum class PageSegmentationModes
    {
        OSD_ONLY = 0,
        AUTO_WITH_OSD,
        AUTO_WITH_NO_OSD,
        DEFAULT,
        SINGLE_COL_TEXT_OF_VAR_SIZE,
        SINGLE_UNIFORM_BLOCK_OF_VERTICALLY_TEXT,
        SINGLE_UNIFORM_BLOCK_TEXT,
        SINGLE_TEXT_LINE,
        SINGLE_WORD,
        SINGLE_WORD_IN_CIRCLE,
        SINGLE_CHARACTER,
        SPARSE_TEXT,
        SPARSE_WITH_OSD,
        RAW_LINE
    };

    enum class EngineModes
    {
        LEGACY_ENGINE_ONLY = 0,
        NEURAL_NETS_LSTM_ONLY,
        LEGACY_LSTM_ENGINES,
        DEFAULT
    };

public:

    OcrOptions()  = default;
    ~OcrOptions() = default;

public:

    static QMap<PageSegmentationModes, QPair<QString, QString> > psmNames();
    static QMap<EngineModes,           QPair<QString, QString> > oemNames();

public:

    QString PsmCodeToValue(PageSegmentationModes _psm)   const;
    QString OemCodeToValue(EngineModes _oem)             const;

public:

    int             psm             = int(PageSegmentationModes::DEFAULT); ///< Page segmentation mode.
    int             oem             = int(EngineModes::DEFAULT);           ///< OCR Engine mode
    int             dpi             = 300;                                 ///< Dot per inch.of input images.
    bool            isSaveTextFile  = true;                                ///< If true, save recognized text to text file.
    bool            isSaveXMP       = true;                                ///< If true, save recognized text to image XMP metadata alternative language tags.

    /**
     * ISO 639-2 3 letters Language code to use while performing OCR on images.
     * https://en.wikipedia.org/wiki/List_of_ISO_639-2_codes
     */
    QString         language;

    QString         tesseractPath;              ///< Path to tesseract binary program.

    QStringList     translations;               ///< List of translation codes to localize recognized text.

    DInfoInterface* iface           = nullptr;  ///< Host application interface.

    bool            multicores      = false;    ///< Process files in parallel.
};

} // namespace DigikamGenericTextConverterPlugin
