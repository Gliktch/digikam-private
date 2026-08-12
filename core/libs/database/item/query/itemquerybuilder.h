/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QVariant>
#include <QString>

// Local includes

#include "coredbsearchxml.h"
#include "itemqueryposthooks.h"
#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemQueryBuilder
{
public:

    ItemQueryBuilder();

    QString buildQuery(const QString& q, QList<QVariant>* boundValues, ItemQueryPostHooks* const hooks) const;
    QString buildQueryFromUrl(const QUrl& url, QList<QVariant>* boundValues) const;
    QString buildQueryFromXml(const QString& xml, QList<QVariant>* boundValues, ItemQueryPostHooks* const hooks) const;
    QString convertFromUrlToXml(const QUrl& url) const;

    /**
     * Use for special queries where ImageTagProperties table is JOIN'ed.
     * (Default: false)
     */
    void setImageTagPropertiesJoined(bool isJoined);

public:

    static void addSqlOperator(QString& sql, SearchXml::Operator op, bool isFirst);
    static void addSqlRelation(QString& sql, SearchXml::Relation rel);
    static void addNoEffectContent(QString& sql, SearchXml::Operator op);

protected:

    void buildGroup(QString& sql, SearchXmlCachingReader& reader,
                    QList<QVariant>* boundValues, ItemQueryPostHooks* const hooks) const;
    bool buildField(QString& sql, SearchXmlCachingReader& reader, const QString& name,
                    QList<QVariant>* boundValues, ItemQueryPostHooks* const hooks) const;

    QString possibleDate(const QString& str, bool& exact) const;

protected:

    QString m_longMonths[12];
    QString m_shortMonths[12];
    bool    m_imageTagPropertiesJoined = false;
};

} // namespace Digikam
