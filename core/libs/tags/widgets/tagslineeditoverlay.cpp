/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rating icon view item at mouse hover
 *
 * SPDX-FileCopyrightText: 2008      by Peter Penz <peter dot penz at gmx dot at>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagslineeditoverlay.h"

// Local includes

#include "digikam_debug.h"
#include "itemdelegate.h"
#include "itemmodel.h"
#include "addtagslineedit.h"
#include "tagscache.h"
#include "abstractalbummodel.h"
#include "albummodel.h"

namespace Digikam
{

TagsLineEditOverlay::TagsLineEditOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

AddTagsLineEdit* TagsLineEditOverlay::addTagsLineEdit() const
{
    return static_cast<AddTagsLineEdit*>(m_widget);
}

QWidget* TagsLineEditOverlay::createWidget()
{
    AddTagsLineEdit* const lineEdit = new AddTagsLineEdit(parentWidget());
    lineEdit->setPlaceholderText(QLatin1String("Name"));
    lineEdit->setReadOnly(false);

    TagModel* const model           = new TagModel(AbstractAlbumModel::IncludeRootAlbum, this);
    model->setCheckable(true);
    model->setRootCheckable(false);
    lineEdit->setSupportingTagModel(model);

    return lineEdit;
}

void TagsLineEditOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(addTagsLineEdit(), SIGNAL(returnPressed(QString)),
                this, SLOT(slotTagChanged(QString)));

        if (view()->model())
        {
            connect(view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
        }
    }
    else
    {
        // widget is deleted

        if (view() && view()->model())
        {
            disconnect(view()->model(), nullptr, this, nullptr);
        }
    }
}

void TagsLineEditOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void TagsLineEditOverlay::hide()
{
/*
    delegate()->setRatingEdited(QModelIndex());
*/
    AbstractWidgetDelegateOverlay::hide();

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Hide called, probably mouse left";

    if (!m_widget->hasFocus())
    {
        m_widget->releaseKeyboard();
        m_widget->releaseMouse();
        addTagsLineEdit()->clear();
    }
}

void TagsLineEditOverlay::updatePosition()
{
    if (!m_index.isValid() || !m_widget)
    {
        return;
    }

    QRect thumbrect  = itemDelegate()->ratingRect();

    //qCDebug(DIGIKAM_GENERAL_LOG) << "updatePosition called, probably a mouseover : " << thumbrect;

    QRect rect       = thumbrect;

    if (rect.width() > addTagsLineEdit()->width())
    {
        int offset = (rect.width() - addTagsLineEdit()->width()) / 2;
        rect.adjust(offset, 0, -offset, 0);
    }

    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 2, rect.height() + 5);
    m_widget->move(rect.topLeft());
    m_widget->setFocus();
}

void TagsLineEditOverlay::updateTag()
{
    if (!m_index.isValid())
    {
        return;
    }

    ItemInfo info = ItemModel::retrieveItemInfo(m_index);
    qCDebug(DIGIKAM_GENERAL_LOG) << "called updateTag()";
/*
    TODO: ADD ratingWidget()->setRating(info.rating());
*/
}

void TagsLineEditOverlay::slotTagChanged(int tagId)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Tag changed";

    if (m_widget && m_widget->isVisible() && m_index.isValid())
    {
        Q_EMIT signalTagEditedById(m_index, tagId);
    }
}

void TagsLineEditOverlay::slotTagChanged(const QString& name)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Tag changed";

    if (m_widget && m_widget->isVisible() && m_index.isValid())
    {
        Q_EMIT signalTagEditedByKeyword(m_index, name);
    }
}

void TagsLineEditOverlay::slotDataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
/*
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
    {
        updateTag();
    }
*/
}

void TagsLineEditOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    // see bug #228810, this is a small workaround

    if (m_widget && m_widget->isVisible() && m_index.isValid() && (index == m_index))
    {
        addTagsLineEdit()->setVisible(true);
    }

    m_index = index;

    updatePosition();
/*
    updateTag();
    delegate()->setRatingEdited(m_index);
*/
    view()->update(m_index);
}

} // namespace Digikam

#include "moc_tagslineeditoverlay.cpp"
