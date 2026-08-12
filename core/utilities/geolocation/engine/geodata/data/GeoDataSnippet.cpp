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

#include "GeoDataSnippet.h"

namespace Marble
{

GeoDataSnippet::GeoDataSnippet(const QString& text, int maxLines)
    : m_text(text),
      m_maxLines(maxLines)
{
    // nothing to do
}

bool GeoDataSnippet::operator==(const GeoDataSnippet& other) const
{
    return m_text == other.m_text && m_maxLines == other.m_maxLines;
}

bool GeoDataSnippet::operator!=(const GeoDataSnippet& other) const
{
    return !this->operator==(other);
}

int GeoDataSnippet::maxLines() const
{
    return m_maxLines;
}

void GeoDataSnippet::setMaxLines(int lines)
{
    m_maxLines = lines;
}

QString GeoDataSnippet::text() const
{
    return m_text;
}

void GeoDataSnippet::setText(const QString& text)
{
    m_text = text;
}

} // namespace Marble

