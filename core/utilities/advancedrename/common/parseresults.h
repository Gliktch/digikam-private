/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a parse results map for token management
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPair>
#include <QString>
#include <QMultiMap>

namespace Digikam
{

class ParseResults
{
public:

    typedef QPair<int, int>                     ResultsKey;
    typedef QPair<QString, QString>             ResultsValue;
    typedef QMultiMap<ResultsKey, ResultsValue> ResultsMap;

public:

    ParseResults()  = default;
    ~ParseResults() = default;

    void addEntry(const ResultsKey& key, const ResultsValue& value);
    void deleteEntry(const ResultsKey& key);

    QList<ResultsKey>   keys()                              const;
    QList<ResultsValue> values()                            const;

    bool       hasKey(const ResultsKey& key);

    QString    result(const ResultsKey& key)                const;
    QString    token(const ResultsKey& key)                 const;

    int        offset(const ResultsKey& key)                const;

    ResultsKey keyAtPosition(int pos)                       const;
    bool       hasKeyAtPosition(int pos)                    const;

    ResultsKey keyAtApproximatePosition(int pos)            const;
    bool       hasKeyAtApproximatePosition(int pos)         const;

    bool       isEmpty()                                    const;

    void       append(const ParseResults &results);
    void       clear();

    QString    replaceTokens(const QString& markedString)   const;

    QString    resultValuesAsString()                       const;

    void       debug()                                      const;

private:

    ResultsKey createInvalidKey()                           const;
    bool       keyIsValid(const ResultsKey& key)            const;

private:

    ResultsMap  m_results;
};

} // namespace Digikam
