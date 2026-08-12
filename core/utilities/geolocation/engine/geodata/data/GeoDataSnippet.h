/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoDataSnippet
{
public:

    /**
     * Create a new snippet with the given @p text and @p maxLines maximum of lines.
     *
     * @param text the text
     * @param maxLines the maximum number of lines
     */
    explicit GeoDataSnippet(const QString& text = QString(), int maxLines = 0);

    /**
     * Check for equality/inequality between two GeoDataSnippets.
     */
    bool operator==(const GeoDataSnippet& other) const;
    bool operator!=(const GeoDataSnippet& other) const;

    /**
     * Return the number of lines that should be displayed at maximum. The value
     * 0 (default) means "all"
     */
    int maxLines() const;

    /**
     * Set the number of lines displayed at maximum.
     */
    void setMaxLines(int lines);

    /**
     * Returns the text that is associated with this snippet.
     */
    QString text() const;

    /**
     * Set the text that the snippet will display.
     */
    void setText(const QString& text);

private:

    QString   m_text;     // Text of the snippet
    int       m_maxLines; // max of lines that are displayed
};

} // namespace Marble
