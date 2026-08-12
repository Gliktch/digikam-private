/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View - Initialization.
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

void LabelsTreeView::initTreeView()
{
    d->itemIterator = QTreeWidgetItemIterator(this);

    setIconSize(QSize(d->iconSizeFromSetting * 5,
                      d->iconSizeFromSetting));

    initRatingsTree();
    initPicksTree();
    initColorsTree();
    expandAll();
    setRootIsDecorated(false);
}

void LabelsTreeView::initRatingsTree()
{
    d->ratings = getOrCreateItem(nullptr);
    d->ratings->setText(0, i18nc("@item: rating tree", "Rating"));
    d->ratings->setFont(0, d->regularFont);
    d->ratings->setFlags(Qt::ItemIsEnabled);

    QTreeWidgetItem* const noRate = getOrCreateItem(d->ratings);
    noRate->setText(0, i18nc("@item: rating tree", "No Rating"));
    noRate->setFont(0, d->regularFont);
    QPixmap pix2(goldenStarPixmap().size());
    pix2.fill(Qt::transparent);
    QPainter p2(&pix2);
    p2.setRenderHint(QPainter::Antialiasing, true);
    p2.setPen(palette().color(QPalette::Active, foregroundRole()));
    p2.drawPixmap(0, 0, goldenStarPixmap(false));
    noRate->setIcon(0, QIcon(pix2));
    noRate->setSizeHint(0, d->iconSize);

    for (int rate = 1 ; rate <= 5 ; ++rate)
    {
        QTreeWidgetItem* const rateWidget = getOrCreateItem(d->ratings);

        QPixmap pix(goldenStarPixmap().width() * rate, goldenStarPixmap().height());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        int offset = 0;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(palette().color(QPalette::Active, foregroundRole()));

        for (int i = 0 ; i < rate ; ++i)
        {
            p.drawPixmap(offset, 0, goldenStarPixmap());
            offset += goldenStarPixmap().width();
        }

        rateWidget->setIcon(0, QIcon(pix));
        rateWidget->setSizeHint(0, d->iconSize);
    }
}

void LabelsTreeView::initPicksTree()
{
    d->picks = getOrCreateItem(nullptr);
    d->picks->setText(0, i18nc("@title: pick tree", "Pick"));
    d->picks->setFont(0, d->regularFont);
    d->picks->setFlags(Qt::ItemIsEnabled);

    for (int i = FirstPickLabel ; i <= LastPickLabel ; i++)
    {
        QTreeWidgetItem* const pickWidgetItem = getOrCreateItem(d->picks);
        pickWidgetItem->setText(0, PickLabelWidget::labelPickName((PickLabel)i));
        pickWidgetItem->setFont(0, d->regularFont);
        pickWidgetItem->setIcon(0, PickLabelWidget::buildIcon((PickLabel)i).pixmap(64, 64));
    }
}

void LabelsTreeView::initColorsTree()
{
    QMap<int, QString> map = TagsActionMngr::defaultManager()->colorLabelNames();
    d->colors              = getOrCreateItem(nullptr);
    d->colors->setText(0, i18nc("@item: color tree", "Color"));
    d->colors->setFont(0, d->regularFont);
    d->colors->setFlags(Qt::ItemIsEnabled);

    ColorLabelDelegate* const delegate = new ColorLabelDelegate(32, this);
    const QList<int> keys              = map.keys();

    for (int label : keys)
    {
        QTreeWidgetItem* const colorWidgetItem = getOrCreateItem(d->colors);
        colorWidgetItem->setData(0, Qt::UserRole, label);
        colorWidgetItem->setFont(0, d->regularFont);
        colorWidgetItem->setText(0, map.value(label));

        setItemDelegateForRow(indexFromItem(colorWidgetItem).row(), delegate);

        if (label == NoColorLabel)
        {
            colorWidgetItem->setIcon(0, QIcon(QIcon::fromTheme(QLatin1String("emblem-unmounted"))
                                        .pixmap(64, 64, QIcon::Disabled)));
        }
        else
        {
            QPixmap colorIcon = colorRectPixmap(QColor(TagsActionMngr::colorSet().value(label)));
            colorWidgetItem->setIcon(0, QIcon(colorIcon));
            colorWidgetItem->setSizeHint(0, d->iconSize);
        }

        if (!d->isCheckableTreeView)
        {
            colorWidgetItem->setFlags(colorWidgetItem->flags() | Qt::ItemIsEditable);
        }
    }

    connect(this, &QTreeWidget::itemChanged,
            [this](QTreeWidgetItem* changedItem, int column)
        {
            if (changedItem && (changedItem->parent() == d->colors) && (column == 0))
            {
                Q_EMIT signalColorNameChanged(changedItem->data(
                                                                0,
                                                                Qt::UserRole).toInt(),
                                                                changedItem->text(0)
                                                               );
            }
        }
    );
}

void LabelsTreeView::restoreSelectionFromHistory(QHash<Labels, QList<int> > neededLabels)
{
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);

    while (*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    for (int rateItemIndex : std::as_const(neededLabels[Ratings]))
    {
        d->ratings->child(rateItemIndex)->setSelected(true);
    }

    for (int pickItemIndex : std::as_const(neededLabels[Picks]))
    {
        d->picks->child(pickItemIndex)->setSelected(true);
    }

    for (int colorItemIndex : std::as_const(neededLabels[Colors]))
    {
        d->colors->child(colorItemIndex)->setSelected(true);
    }
}

} // namespace Digikam
