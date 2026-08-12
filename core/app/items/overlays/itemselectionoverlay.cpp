/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : selection icon view item at mouse hover
 *
 * SPDX-FileCopyrightText: 2008      by Peter Penz <peter dot penz at gmx dot at>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemselectionoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemcategorizedview.h"

namespace Digikam
{

ItemSelectionOverlayButton::ItemSelectionOverlayButton(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
    setup();
}

QSize ItemSelectionOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ItemSelectionOverlayButton::icon()
{
    return QIcon::fromTheme(isChecked() ? QLatin1String("spinbox-decrease")
                                        : QLatin1String("list-add"));
}

void ItemSelectionOverlayButton::updateToolTip()
{
    setToolTip(isChecked() ? i18nc("@info:tooltip", "Deselect Item")
                           : i18nc("@info:tooltip", "Select Item"));
}

// --------------------------------------------------------------------

ItemSelectionOverlay::ItemSelectionOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ItemSelectionOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));

        connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
    }
    else
    {
        // button is deleted

        if (m_view)
        {
            disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                       this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
        }
    }
}

ItemViewHoverButton* ItemSelectionOverlay::createButton()
{
    return new ItemSelectionOverlayButton(view());
}

void ItemSelectionOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.left() + gap;
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));

    QItemSelectionModel* const selModel = m_view->selectionModel();
    button()->setChecked(selModel->isSelected(index));
}

void ItemSelectionOverlay::slotClicked(bool checked)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        QItemSelectionModel* const selModel = m_view->selectionModel();

        if (checked)
        {
            selModel->select(index, QItemSelectionModel::Select);
        }
        else
        {
            selModel->select(index, QItemSelectionModel::Deselect);
        }

        selModel->setCurrentIndex(index, QItemSelectionModel::Current);
    }
}

void ItemSelectionOverlay::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        if (selected.contains(index))
        {
            button()->setChecked(true);
        }
        else if (deselected.contains(index))
        {
            button()->setChecked(false);
        }
    }
}

} // namespace Digikam

#include "moc_itemselectionoverlay.cpp"
