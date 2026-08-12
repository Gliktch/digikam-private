/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *               Internal containers.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemquerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>
#include <QUrl>
#include <QLocale>
#include <QUrlQuery>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "metaengine.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "tagscache.h"
#include "coredbbackend.h"
#include "fieldquerybuilder.h"

namespace Digikam
{

class Q_DECL_HIDDEN RuleTypeForConversion
{
public:

    RuleTypeForConversion() = default;

public:

    QString             key;
    SearchXml::Relation op  = SearchXml::Equal;
    QString             val;
};

// -------------------------------------------------------------------------

enum SKey
{
    ALBUM = 0,
    ALBUMNAME,
    ALBUMCAPTION,
    ALBUMCOLLECTION,
    TAG,
    TAGNAME,
    IMAGENAME,
    IMAGECAPTION,
    IMAGEDATE,
    KEYWORD,
    RATING
};

enum SOperator
{
    EQ = 0,
    NE,
    LT,
    GT,
    LIKE,
    NLIKE,
    LTE,
    GTE
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN RuleType
{
public:

    SKey      key = ALBUM;
    SOperator op  = EQ;
    QString   val;
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN SubQueryBuilder
{
public:

    QString build(enum SKey key,
                  enum SOperator op,
                  const QString& passedVal,
                  QList<QVariant>* boundValues) const;
};

} // namespace Digikam
