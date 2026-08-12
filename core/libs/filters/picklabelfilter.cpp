/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-16
 * Description : pick label filter
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "picklabelfilter.h"

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "tagscache.h"

namespace Digikam
{

PickLabelFilter::PickLabelFilter(QWidget* const parent)
    : PickLabelWidget(parent)
{
    setDescriptionBoxVisible(false);
    setButtonsExclusive(false);
    reset();

    connect(this, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotPickLabelSelectionChanged()));
}

void PickLabelFilter::reset()
{
    setPickLabels(QList<PickLabel>());
    slotPickLabelSelectionChanged();
}

QList<TAlbum*> PickLabelFilter::getCheckedPickLabelTags()
{
    QList<TAlbum*> list;
    int tagId         = 0;
    TAlbum* tag       = nullptr;
    const auto labels = pickLabels();

    for (const PickLabel& pl : labels)
    {
        tagId = TagsCache::instance()->tagForPickLabel(pl);
        tag   = AlbumManager::instance()->findTAlbum(tagId);

        if (tagId)
        {
            list.append(tag);
        }
    }

    return list;
}

void PickLabelFilter::slotPickLabelSelectionChanged()
{
    Q_EMIT signalPickLabelSelectionChanged(pickLabels());
}

} // namespace Digikam

#include "moc_picklabelfilter.cpp"
