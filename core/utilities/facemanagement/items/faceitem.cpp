/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Face tag region item
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "faceitem.h"

// Local includes

#include "digikam_debug.h"
#include "assignnamewidgetstates.h"

namespace Digikam
{

FaceItem::FaceItem(QGraphicsItem* const parent)
    : RegionFrameItem(parent)
{
}

void FaceItem::setFace(const FaceTagsIface& face)
{
    m_face = face;
    updateCurrentTag();
    setEditable(!(m_face.isIgnoredName()  ||
                  m_face.isConfirmedName()));
}

FaceTagsIface FaceItem::face() const
{
    return m_face;
}

void FaceItem::setHudWidget(AssignNameWidget* const widget)
{
    m_widget = widget;
    updateCurrentTag();
    RegionFrameItem::setHudWidget(widget);

    // Ensure that all HUD widgets are stacked before the frame items.

    hudWidget()->setZValue(1);
}

AssignNameWidget* FaceItem::widget() const
{
    return m_widget;
}

void FaceItem::switchMode(AssignNameWidget::Mode mode)
{
    if (!m_widget || (m_widget->mode() == mode))
    {
        return;
    }

    if (!m_changer)
    {
        m_changer = new AssignNameWidgetStates(this);
    }

    m_changer->changeValue(mode);
}

void FaceItem::setEditable(bool allowEdit)
{
    changeFlags(ShowResizeHandles | MoveByDrag, allowEdit);
}

void FaceItem::updateCurrentTag()
{
    if (m_widget)
    {
        m_widget->setCurrentFace(m_face);
    }
}

} // namespace Digikam

#include "moc_faceitem.cpp"
