/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : OCR Tesseract engine
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QObject>
#include <QUrl>

// Local includes

#include "ocroptions.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class OcrTesseractEngine : public QObject
{
    Q_OBJECT

public:

    enum ConvertError
    {
        PROCESS_CONTINUE =  1,        ///< Current stages is done.
        PROCESS_COMPLETE =  0,        ///< All stages done.
        PROCESS_FAILED   = -1,        ///< A failure happen while processing.
        PROCESS_CANCELED = -2,        ///< User has canceled processing.
    };

public:

    explicit OcrTesseractEngine(QObject* const parent = nullptr);
    ~OcrTesseractEngine();

    QString inputFile()          const;
    QString outputFile()         const;
    QString outputText()         const;

    void setInputFile(const QString& filePath);
    void setOutputFile(const QString& filePath);

    OcrOptions ocrOptions()      const;
    void setOcrOptions(const OcrOptions& opt);

public:

    static void translate(MetaEngine::AltLangMap& commentsMap,
                          const QStringList& langs);

    static void saveTextFile(const QString& inFile,
                             QString& outFile,
                             const MetaEngine::AltLangMap& commentsMap);

    static void saveXMP(const QUrl& url,
                        const MetaEngine::AltLangMap& commentsMap,
                        DInfoInterface* const iface);

    int  runOcrProcess();

    void cancelOcrProcess();

private:

    /// @note disabled

    OcrTesseractEngine(const OcrTesseractEngine&)            = delete;
    OcrTesseractEngine& operator=(const OcrTesseractEngine&) = delete;

private:

    void saveOcrResult();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTextConverterPlugin
