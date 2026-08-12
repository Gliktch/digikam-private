/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View - Settings.
 *
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "labelstreeview_p.h"

namespace Digikam
{

QHash<LabelsTreeView::Labels, QList<int> > LabelsTreeView::selectedLabels()
{
    QHash<Labels, QList<int> > selectedLabelsHash;
    QList<int> selectedRatings;
    QList<int> selectedPicks;
    QList<int> selectedColors;

    if (d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

        while (*it)
        {
            QTreeWidgetItem* const item = (*it);

            if      (item->parent() == d->ratings)
            {
                selectedRatings << indexFromItem(item).row();
            }
            else if (item->parent() == d->picks)
            {
                selectedPicks << indexFromItem(item).row();
            }
            else
            {
                selectedColors << indexFromItem(item).row();
            }

            ++it;
        }
    }
    else
    {
        const auto items = selectedItems();

        for (QTreeWidgetItem* const item : items)
        {
            if      (item->parent() == d->ratings)
            {
                selectedRatings << indexFromItem(item).row();
            }
            else if (item->parent() == d->picks)
            {
                selectedPicks << indexFromItem(item).row();
            }
            else
            {
                selectedColors << indexFromItem(item).row();
            }
        }
    }

    selectedLabelsHash[Ratings] = selectedRatings;
    selectedLabelsHash[Picks]   = selectedPicks;
    selectedLabelsHash[Colors]  = selectedColors;

    return selectedLabelsHash;
}

void LabelsTreeView::doLoadState()
{
    d->isLoadingState                = true;
    KConfigGroup configGroup         = getConfigGroup();
    const QList<int> expansion       = configGroup.readEntry(entryName(d->configExpansionEntry),       QList<int>());
    const QList<int> selectedRatings = configGroup.readEntry(entryName(d->configRatingSelectionEntry), QList<int>());
    const QList<int> selectedPicks   = configGroup.readEntry(entryName(d->configPickSelectionEntry),   QList<int>());
    const QList<int> selectedColors  = configGroup.readEntry(entryName(d->configColorSelectionEntry),  QList<int>());

    d->ratings->setExpanded(true);
    d->picks->setExpanded(true);
    d->colors->setExpanded(true);

    for (int parent : std::as_const(expansion))
    {
        switch (parent)
        {
            case 1:
            {
                d->ratings->setExpanded(false);
                break;
            }

            case 2:
            {
                d->picks->setExpanded(false);
                break;
            }

            case 3:
            {
                d->colors->setExpanded(false);
            }

            default:
            {
                break;
            }
        }
    }

    for (int rating : std::as_const(selectedRatings))
    {
        if (d->isCheckableTreeView)
        {
            d->ratings->child(rating)->setCheckState(0, Qt::Checked);
        }
        else
        {
            d->ratings->child(rating)->setSelected(true);
        }
    }

    for (int pick : std::as_const(selectedPicks))
    {
        if (d->isCheckableTreeView)
        {
            d->picks->child(pick)->setCheckState(0, Qt::Checked);
        }
        else
        {
            d->picks->child(pick)->setSelected(true);
        }
    }

    for (int color : std::as_const(selectedColors))
    {
        if (d->isCheckableTreeView)
        {
            d->colors->child(color)->setCheckState(0, Qt::Checked);
        }
        else
        {
            d->colors->child(color)->setSelected(true);
        }
    }

    d->isLoadingState = false;
}

void LabelsTreeView::doSaveState()
{
    KConfigGroup configGroup = getConfigGroup();
    QList<int> expansion;

    if (!d->ratings->isExpanded())
    {
        expansion << 1;
    }

    if (!d->picks->isExpanded())
    {
        expansion << 2;
    }

    if (!d->colors->isExpanded())
    {
        expansion << 3;
    }

    QHash<Labels, QList<int> > labels = selectedLabels();

    configGroup.writeEntry(entryName(d->configExpansionEntry),       expansion);
    configGroup.writeEntry(entryName(d->configRatingSelectionEntry), labels[Ratings]);
    configGroup.writeEntry(entryName(d->configPickSelectionEntry),   labels[Picks]);
    configGroup.writeEntry(entryName(d->configColorSelectionEntry),  labels[Colors]);
}

void LabelsTreeView::slotSettingsChanged()
{
    if (d->iconSizeFromSetting != ApplicationSettings::instance()->getTreeViewIconSize())
    {
        d->iconSizeFromSetting = ApplicationSettings::instance()->getTreeViewIconSize();
        setIconSize(QSize(d->iconSizeFromSetting * 5, d->iconSizeFromSetting));
        d->iconSize            = QSize(d->iconSizeFromSetting, d->iconSizeFromSetting);
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            (*it)->setSizeHint(0, d->iconSize);
            ++it;
        }
    }

    if (d->regularFont != ApplicationSettings::instance()->getTreeViewFont())
    {
        d->regularFont = ApplicationSettings::instance()->getTreeViewFont();
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            (*it)->setFont(0, d->regularFont);
            ++it;
        }
    }
}

} // namespace Digikam
