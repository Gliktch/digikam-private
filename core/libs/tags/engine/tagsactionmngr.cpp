/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsactionmngr_p.h"

namespace Digikam
{

TagsActionMngr* TagsActionMngr::m_defaultManager = nullptr;

TagsActionMngr* TagsActionMngr::defaultManager()
{
    return m_defaultManager;
}

TagsActionMngr::TagsActionMngr(QWidget* const parent)
    : QObject(parent),
      d      (new Private)
{
    if (!m_defaultManager)
    {
        m_defaultManager = this;
    }

    loadColorNames();

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
}

TagsActionMngr::~TagsActionMngr()
{
    saveColorNames();

    delete d;

    if (m_defaultManager == this)
    {
        m_defaultManager = nullptr;
    }
}

void TagsActionMngr::slotAlbumDeleted(Album* album)
{
    const TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (!talbum)
    {
        return;
    }

    if (removeTagActionShortcut(talbum->id()))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Delete Shortcut assigned to tag " << album->id();
    }
}

} // namespace Digikam

#include "moc_tagsactionmngr.cpp"
