/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-09
 * Description : color label filter
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorlabelfilter.h"

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "tagsactionmngr.h"
#include "tagscache.h"

namespace Digikam
{

ColorLabelFilter::ColorLabelFilter(QWidget* const parent)
    : ColorLabelWidget(parent)
{
    setDescriptionBoxVisible(false);
    setButtonsExclusive(false);
    reset();

    connect(this, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotColorLabelSelectionChanged()));

    connect(TagsActionMngr::defaultManager(), &TagsActionMngr::signalTagsActionCollectionsRegistered,
            this, [this]()
       {
           setDescriptionBoxVisible(false);
       }
    );

    connect(TagsActionMngr::defaultManager(), &TagsActionMngr::signalColorLabelNamesUpdated,
            this, [this]()
       {
           setDescriptionBoxVisible(false);
       }
    );
}

void ColorLabelFilter::reset()
{
    setColorLabels(QList<ColorLabel>());
    slotColorLabelSelectionChanged();
}

QList<TAlbum*> ColorLabelFilter::getCheckedColorLabelTags()
{
    QList<TAlbum*> list;
    int tagId         = 0;
    TAlbum* tag       = nullptr;
    const auto labels = colorLabels();

    for (const ColorLabel& cl : labels)
    {
        tagId = TagsCache::instance()->tagForColorLabel(cl);
        tag   = AlbumManager::instance()->findTAlbum(tagId);

        if (tagId && tag)
        {
            list.append(tag);
        }
    }

    return list;
}

void ColorLabelFilter::slotColorLabelSelectionChanged()
{
    Q_EMIT signalColorLabelSelectionChanged(colorLabels());
}

} // namespace Digikam

#include "moc_colorlabelfilter.cpp"
