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

SearchFieldLabels::SearchFieldLabels(QObject* const parent)
    : SearchField(parent)
{
}

void SearchFieldLabels::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    QHBoxLayout* const hbox = new QHBoxLayout;
    m_pickLabelFilter       = new PickLabelFilter;
    m_colorLabelFilter      = new ColorLabelFilter;
    hbox->addWidget(m_pickLabelFilter);
    hbox->addStretch(10);
    hbox->addWidget(m_colorLabelFilter);

    connect(m_pickLabelFilter, SIGNAL(signalPickLabelSelectionChanged(QList<PickLabel>)),
            this, SLOT(updateState()));

    connect(m_colorLabelFilter, SIGNAL(signalColorLabelSelectionChanged(QList<ColorLabel>)),
            this, SLOT(updateState()));

    updateState();

    layout->addLayout(hbox, row, column, 1, 3);
}

void SearchFieldLabels::updateState()
{
    setValidValueState(!m_colorLabelFilter->colorLabels().isEmpty());
}

void SearchFieldLabels::read(SearchXmlCachingReader& reader)
{
    const TAlbum* a = nullptr;
    QList<int> ids  = reader.valueToIntOrIntList();
    QList<ColorLabel> clabels;
    QList<PickLabel>  plabels;

    for (int id : std::as_const(ids))
    {
        a = AlbumManager::instance()->findTAlbum(id);

        if (!a)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Search: Did not find Label album for ID" << id << "given in Search XML";
        }
        else
        {
            int cl = TagsCache::instance()->colorLabelForTag(a->id());

            if (cl != -1)
            {
                clabels.append((ColorLabel)cl);
            }
            else
            {
                int pl = TagsCache::instance()->pickLabelForTag(a->id());

                if (pl != -1)
                {
                    plabels.append((PickLabel)pl);
                }
            }
        }
    }

    m_colorLabelFilter->setColorLabels(clabels);
    m_pickLabelFilter->setPickLabels(plabels);
}

void SearchFieldLabels::write(SearchXmlWriter& writer)
{
    QList<int>     albumIds;
    QList<TAlbum*> clAlbums = m_colorLabelFilter->getCheckedColorLabelTags();

    if (!clAlbums.isEmpty())
    {
        for (const TAlbum* const album : std::as_const(clAlbums))
        {
            albumIds << album->id();
        }
    }

    QList<TAlbum*> plAlbums = m_pickLabelFilter->getCheckedPickLabelTags();

    if (!plAlbums.isEmpty())
    {
        for (const TAlbum* const album : std::as_const(plAlbums))
        {
            albumIds << album->id();
        }
    }

    if (albumIds.isEmpty())
    {
        return;
    }

    // NOTE: As Color Labels are internal tags, we trig database on "tagid"
    //       with "labels" in ItemQueryBuilder::buildField().

    writer.writeField(m_name, SearchXml::InTree);

    if (albumIds.size() > 1)
    {
        writer.writeValue(albumIds);
    }
    else
    {
        writer.writeValue(albumIds.first());
    }

    writer.finishField();
}

void SearchFieldLabels::reset()
{
    m_colorLabelFilter->reset();
    m_pickLabelFilter->reset();
}

void SearchFieldLabels::setValueWidgetsVisible(bool visible)
{
    m_colorLabelFilter->setVisible(visible);
    m_pickLabelFilter->setVisible(visible);
}

QList<QRect> SearchFieldLabels::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_pickLabelFilter->geometry();
    rects << m_colorLabelFilter->geometry();

    return rects;
}

} // namespace Digikam
