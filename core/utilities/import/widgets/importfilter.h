/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Filter for filter combobox
 *
 * SPDX-FileCopyrightText: 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 * SPDX-FileCopyrightText: 2014      by Teemu Rytilahti <tpr@iki.fi>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QHash>
#include <QString>
#include <QList>
#include <QStringList>
#include <QRegularExpression>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

class Filter
{
public:

    Filter()  = default;
    ~Filter() = default;

public:

    QString toString();
    void    fromString(const QString& filter);
    bool    match(const QStringList& wildcards, const QString& _name);
    const   QRegularExpression& regexp(const QString& wildcard);
    const   QStringList& mimeWildcards(const QString& mime);
    bool    matchesCurrentFilter(const CamItemInfo& item);

public:

    QString                            name;
    bool                               onlyNew              = false;
    QStringList                        fileFilter;
    QStringList                        pathFilter;
    QStringList                        ignoreNames;
    QStringList                        ignoreExtensions;
    QString                            mimeFilter;
    QHash<QString, QRegularExpression> filterHash;
    QHash<QString, QStringList>        mimeHash;
};

typedef QList<Filter*> FilterList;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Filter*)
