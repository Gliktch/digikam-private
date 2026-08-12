/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : database SQL queries helper class
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2026      by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "coredbsearchxml.h"

namespace Digikam
{

class ItemQueryPostHooks;

class Q_DECL_HIDDEN FieldQueryBuilder
{
public:

    FieldQueryBuilder(QString& sql,
                      SearchXmlCachingReader& reader,
                      QList<QVariant>* boundValues,
                      ItemQueryPostHooks* const hooks,
                      SearchXml::Relation relation);

public:

    QString&                sql;
    SearchXmlCachingReader& reader;
    QList<QVariant>*        boundValues;
    ItemQueryPostHooks*     hooks       = nullptr;
    SearchXml::Relation     relation    = SearchXml::Equal;

public:

    QString prepareForLike(const QString& str)                  const;

    void addIntField(const QString& name);
    void addLongField(const QString& name);
    void addDoubleField(const QString& name);
    void addStringField(const QString& name);
    void addDateField(const QString& name);
    void addRecentDateField(const QString& name, int seconds);
    void addChoiceIntField(const QString& name);
    void addLongListField(const QString& name);
    void addIntBitmaskField(const QString& name);
    void addChoiceStringField(const QString& name);
    void addPosition();
    void addRectanglePositionSearch(double lon1, double lat1,
                                    double lon2, double lat2)   const;
};

} // namespace Digikam
