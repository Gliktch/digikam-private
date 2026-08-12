/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchfields_p.h"

namespace Digikam
{

SearchFieldKeyword::SearchFieldKeyword(QObject* const parent)
    : SearchFieldText(parent)
{
}

void SearchFieldKeyword::read(SearchXmlCachingReader& reader)
{
    QString keyword = reader.value();
    m_edit->setText(KeywordSearch::merge(m_edit->text(), keyword));
}

void SearchFieldKeyword::write(SearchXmlWriter& writer)
{
    QStringList keywordList = KeywordSearch::split(m_edit->text());

    for (const QString& keyword : std::as_const(keywordList))
    {
        if (!keyword.isEmpty())
        {
            writer.writeField(m_name, SearchXml::Like);
            writer.writeValue(keyword);
            writer.finishField();
        }
    }
}

} // namespace Digikam
