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

#pragma once

// Qt includes

#include <QMap>
#include <QString>
#include <QDateTime>
#include <QDebug>

// Local includes

#include "digikam_export.h"
#include "metaengine.h"

namespace Digikam
{

class DIGIKAM_EXPORT CaptionValues
{
public:

    CaptionValues()  = default;
    ~CaptionValues() = default;

    bool operator==(const CaptionValues& val) const;

public:

    QString   caption;
    QString   author;
    QDateTime date;
};

//! qDebug() stream operator. Writes values @a val to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const CaptionValues& val);

// --------------------------------------------------------------------

/**
 * A map used to store a list of Alternative Language values + author and date properties
 * The map key is the language code following RFC3066 notation
 * (like "fr-FR" for French), and the CaptionsMap value all caption properties.
 */
class DIGIKAM_EXPORT CaptionsMap : public QMap<QString, CaptionValues>
{
public:

    CaptionsMap()  = default;
    ~CaptionsMap() = default;

    void setData(const MetaEngine::AltLangMap& comments,
                 const MetaEngine::AltLangMap& authors,
                 const QString& commonAuthor,
                 const MetaEngine::AltLangMap& dates);

    void fromAltLangMap(const MetaEngine::AltLangMap& map);
    MetaEngine::AltLangMap toAltLangMap()   const;

    /**
     * Sets the author for the comments in the specified languages.
     * If commonAuthor is not null, it will be used to set the author of all comments
     * for which the author is not specified in the map.
     */
    void setAuthorsList(const MetaEngine::AltLangMap& map, const QString& commonAuthor = QString());
    MetaEngine::AltLangMap authorsList()    const;

    void setDatesList(const MetaEngine::AltLangMap& map);
    MetaEngine::AltLangMap datesList()      const;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::CaptionValues)
Q_DECLARE_METATYPE(Digikam::CaptionsMap)
