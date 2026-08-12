/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-13
 * Description : caption values container
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "captionvalues.h"

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

bool CaptionValues::operator==(const CaptionValues& val) const
{
    bool b1  = (author  == val.author);
    bool b2  = (caption == val.caption);
    // bool b3  = (date    == val.date);

    return (b1 && b2 /*&& b3*/);
}

QDebug operator<<(QDebug dbg, const CaptionValues& val)
{
    dbg.nospace() << "CaptionValues::caption: "
                  << val.caption << ", ";
    dbg.nospace() << "CaptionValues::author: "
                  << val.author << ", ";
    dbg.nospace() << "CaptionValues::date: "
                  << val.date;

    return dbg.space();
}

// --------------------------------------------------------------------

void CaptionsMap::setData(const MetaEngine::AltLangMap& comments,
                          const MetaEngine::AltLangMap& authors,
                          const QString& commonAuthor,
                          const MetaEngine::AltLangMap& dates)
{
    fromAltLangMap(comments);
    setAuthorsList(authors, commonAuthor);
    setDatesList(dates);
}

MetaEngine::AltLangMap CaptionsMap::toAltLangMap() const
{
    MetaEngine::AltLangMap map;

    for (CaptionsMap::const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        map.insert(it.key(), (*it).caption);
    }

    return map;
}

void CaptionsMap::fromAltLangMap(const MetaEngine::AltLangMap& map)
{
    clear();

    for (MetaEngine::AltLangMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        CaptionValues val;
        val.caption = it.value();
        insert(it.key(), val);
    }
}

MetaEngine::AltLangMap CaptionsMap::authorsList() const
{
    MetaEngine::AltLangMap map;

    for (CaptionsMap::const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        map.insert(it.key(), (*it).author);
    }

    return map;
}

void CaptionsMap::setAuthorsList(const MetaEngine::AltLangMap& map, const QString& commonAuthor)
{
    for (CaptionsMap::iterator it = begin() ; it != end() ; ++it)
    {
        MetaEngine::AltLangMap::const_iterator authorIt = map.find(it.key());

        if      (authorIt != map.constEnd())
        {
            (*it).author = authorIt.value();
        }
        else if (!commonAuthor.isNull())
        {
            (*it).author = commonAuthor;
        }
    }
}

MetaEngine::AltLangMap CaptionsMap::datesList() const
{
    MetaEngine::AltLangMap map;

    for (CaptionsMap::const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        map.insert(it.key(), asDateTimeLocal((*it).date).toString(Qt::ISODate));
    }

    return map;
}

void CaptionsMap::setDatesList(const MetaEngine::AltLangMap& map)
{
    for (MetaEngine::AltLangMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        CaptionsMap::iterator val = find(it.key());

        if (val != end())
        {
            (*val).date = asDateTimeLocal(QDateTime::fromString(it.value(), Qt::ISODate));
        }
    }
}

} // namespace Digikam
