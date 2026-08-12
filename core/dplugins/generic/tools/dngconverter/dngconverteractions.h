/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-24
 * Description : DNG converter plugin action descriptions
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QImage>
#include <QMetaType>
#include <QUrl>

// Local includes

#include "dngwriter.h"

using namespace Digikam;

namespace DigikamGenericDNGConverterPlugin
{

enum DNGConverterAction
{
    NONE = 0,
    IDENTIFY,
    PROCESS
};

class DNGConverterActionData
{

public:

    DNGConverterActionData() = default;

    bool                starting = false;
    int                 result   = DNGWriter::PROCESS_COMPLETE;

    QString             destPath;
    QString             message;

    QImage              image;

    QUrl                fileUrl;

    DNGConverterAction  action   = NONE;
};

}  // namespace DigikamGenericDNGConverterPlugin

Q_DECLARE_METATYPE(DigikamGenericDNGConverterPlugin::DNGConverterActionData)
