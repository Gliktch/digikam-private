/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View.
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

LabelsTreeView::LabelsTreeView(QWidget* const parent, bool setCheckable)
    : QTreeWidget      (parent),
      StateSavingObject(this),
      d                (new Private(this))
{
    d->regularFont         = ApplicationSettings::instance()->getTreeViewFont();
    d->iconSizeFromSetting = ApplicationSettings::instance()->getTreeViewIconSize();
    d->iconSize            = QSize(d->iconSizeFromSetting, d->iconSizeFromSetting);
    d->isCheckableTreeView = setCheckable;

    setHeaderLabel(i18nc("@title", "Labels"));
    setUniformRowHeights(false);

    initTreeView();

    if (d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            if ((*it)->parent())
            {
                (*it)->setFlags((*it)->flags()|Qt::ItemIsUserCheckable);
                (*it)->setCheckState(0, Qt::Unchecked);
            }

            ++it;
        }
    }
    else
    {
        setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    // ---

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(this, &LabelsTreeView::signalColorNameChanged,
            TagsActionMngr::defaultManager(), &TagsActionMngr::slotColorNameChanged);

    connect(ThemeManager::instance(), &ThemeManager::signalThemeChanged,
            this, [this]()
        {
            initTreeView();
        }
    );
}

LabelsTreeView::~LabelsTreeView()
{
    delete d;
}

bool LabelsTreeView::isCheckable() const
{
    return d->isCheckableTreeView;
}

bool LabelsTreeView::isLoadingState() const
{
    return d->isLoadingState;
}

QTreeWidgetItem* LabelsTreeView::getOrCreateItem(QTreeWidgetItem* const parent)
{
    QTreeWidgetItem* item = nullptr;

    if (*d->itemIterator)
    {
        item = (*d->itemIterator);
    }
    else
    {
        if (parent)
        {
            item = new QTreeWidgetItem(parent);
        }
        else
        {
            item = new QTreeWidgetItem(this);
        }
    }

    ++d->itemIterator;

    return item;
}

QPixmap LabelsTreeView::goldenStarPixmap(bool fillin) const
{
    QPixmap pixmap = QPixmap(60, 60);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);

    if (fillin)
    {
        QPen pen(palette().color(QPalette::Active, foregroundRole()));
        p1.setBrush(qApp->palette().color(QPalette::Link));
        p1.setPen(pen);
    }
    else
    {
        QPen pen(qApp->palette().color(QPalette::Active, QPalette::ButtonText));
        p1.setPen(pen);
    }

    QTransform transform;
    transform.scale(4, 4);     // 60px/15px (RatingWidget::starPolygon() size is 15*15px)
    p1.setTransform(transform);

    p1.drawPolygon(RatingWidget::starPolygon(), Qt::WindingFill);
    p1.end();

    return pixmap;
}

QPixmap LabelsTreeView::colorRectPixmap(const QColor& color) const
{
    QRect rect(8, 8, 48, 48);
    QPixmap pixmap = QPixmap(60, 60);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(color);
    p1.setPen((color == Qt::black) ? Qt::white : Qt::black);
    p1.drawRect(rect);
    p1.end();

    return pixmap;
}

void LabelsTreeView::setCurrentAlbum()
{
    Q_EMIT signalSetCurrentAlbum();
}

} // namespace Digikam

#include "moc_labelstreeview.cpp"
