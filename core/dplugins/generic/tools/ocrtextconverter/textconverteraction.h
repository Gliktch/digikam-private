/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text converter plugin action descriptions
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QUrl>

// Local includes

#include "ocrtesseractengine.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

enum TextConverterAction
{
    NONE = 0,
    PROCESS
};

class TextConverterActionData
{

public:

    TextConverterActionData() = default;

public:

    bool                 starting   = false;
    int                  result     = OcrTesseractEngine::PROCESS_COMPLETE;

    QString              destPath;
    QString              message;

    QString              outputText;

    QImage               image;

    QUrl                 fileUrl;

    TextConverterAction  action     = NONE;
};

}  // namespace DigikamGenericTextConverterPlugin

Q_DECLARE_METATYPE(DigikamGenericTextConverterPlugin::TextConverterActionData)
