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

#include "facerejectionoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemcategorizedview.h"
#include "itemfacedelegate.h"
#include "itemmodel.h"

namespace Digikam
{

FaceRejectionOverlayButton::FaceRejectionOverlayButton(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
    setup();
}

QSize FaceRejectionOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon FaceRejectionOverlayButton::icon()
{
    return QIcon::fromTheme(QLatin1String("window-close"));
}

void FaceRejectionOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "If this is not a face, click to delete it."));
}

// --------------------------------------------------------------------

FaceRejectionOverlay::FaceRejectionOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void FaceRejectionOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* FaceRejectionOverlay::createButton()
{
    return new FaceRejectionOverlayButton(view());
}

void FaceRejectionOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - gap - size;
    const int y      = rect.top()   + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void FaceRejectionOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        Q_EMIT rejectFaces(affectedIndexes(index));
    }
}

bool FaceRejectionOverlay::checkIndex(const QModelIndex& index) const
{
    return !index.data(ItemModel::ExtraDataRole).isNull();
}

void FaceRejectionOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void FaceRejectionOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam

#include "moc_facerejectionoverlay.cpp"
