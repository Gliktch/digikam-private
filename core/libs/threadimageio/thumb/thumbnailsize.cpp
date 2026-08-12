/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : thumbnails size interface
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbnailsize.h"

// KDE includes

#include <kconfiggroup.h>

namespace Digikam
{

static const QString s_configUseLargeThumbsEntry(QLatin1String("Use Large Thumbs"));
static bool          s_useLargeThumbs = false;

ThumbnailSize::ThumbnailSize(int size)
{
    m_Size = size;
}

ThumbnailSize::ThumbnailSize(const ThumbnailSize& thumbsize)
{
    m_Size = thumbsize.m_Size;
}

ThumbnailSize& ThumbnailSize::operator=(const ThumbnailSize& thumbsize)
{
    m_Size = thumbsize.m_Size;

    return *this;
}

bool ThumbnailSize::operator==(const ThumbnailSize& thumbsize) const
{
    return (m_Size == thumbsize.m_Size);
}

bool ThumbnailSize::operator!=(const ThumbnailSize& thumbsize) const
{
    return (m_Size != thumbsize.m_Size);
}

int ThumbnailSize::size() const
{
    return m_Size;
}

// -- Static methods ---------------------------------------------------------

void ThumbnailSize::setUseLargeThumbs(bool val)
{
    s_useLargeThumbs = val;
}

bool ThumbnailSize::getUseLargeThumbs()
{
    return s_useLargeThumbs;
}

void ThumbnailSize::readSettings(const KConfigGroup& group)
{
    setUseLargeThumbs(group.readEntry(s_configUseLargeThumbsEntry, false));
}

void ThumbnailSize::saveSettings(KConfigGroup& group, bool val)
{
    group.writeEntry(s_configUseLargeThumbsEntry, val);
}

int ThumbnailSize::maxThumbsSize()
{
    if (s_useLargeThumbs)
    {
        return ThumbnailSize::HD;
    }

    return ThumbnailSize::Huge;
}

} // namespace Digikam
