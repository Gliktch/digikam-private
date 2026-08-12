/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-14
 * Description : overlay for assigning names to faces
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Peter Penz <peter dot penz at gmx dot at>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignnameoverlay.h"

// Qt includes

#include <QApplication>
#include <QPushButton>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "assignnamewidget.h"
#include "facetagsiface.h"
#include "facetags.h"
#include "itemdelegate.h"
#include "itemmodel.h"
#include "itemcategorizedview.h"
#include "taggingaction.h"
#include "tagscache.h"
#include "searchutilities.h"
#include "applicationsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN AssignNameOverlay::Private
{
public:

    Private() = default;

    bool isChildWidget(QWidget* widget, const QWidget* const parent) const
    {
        if (!parent)
        {
            return false;
        }

        // isAncestorOf may not work if widgets are located in different windows

        while (widget)
        {
            if (widget == parent)
            {
                return true;
            }

            widget = widget->parentWidget();
        }

        return false;
    }

public:

    TagModel                  tagModel          = TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    CheckableAlbumFilterModel filterModel;
    TagPropertiesFilterModel  filteredModel;

    AssignNameWidget*         assignNameWidget  = nullptr;
};

AssignNameOverlay::AssignNameOverlay(QObject* const parent)
    : PersistentWidgetDelegateOverlay(parent),
      d                              (new Private)
{
    d->filteredModel.setSourceAlbumModel(&d->tagModel);
    d->filterModel.setSourceFilterModel(&d->filteredModel);

    // Restrict the tag properties filter model to people if configured.

    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        if (settings->showOnlyPersonTagsInPeopleSidebar())
        {
            d->filteredModel.listOnlyTagsWithProperty(TagPropertyName::person());
        }
    }
}

AssignNameOverlay::~AssignNameOverlay()
{
    delete d;
}

AssignNameWidget* AssignNameOverlay::assignNameWidget() const
{
    return d->assignNameWidget;
}

QWidget* AssignNameOverlay::createWidget()
{
    DVBox* const vbox    = new DVBox(parentWidget());
    d->assignNameWidget  = new AssignNameWidget(vbox);
    d->assignNameWidget->setMode(AssignNameWidget::UnconfirmedEditMode);
    d->assignNameWidget->setVisualStyle(AssignNameWidget::TranslucentThemedFrameless);
    d->assignNameWidget->setTagEntryWidgetMode(AssignNameWidget::AddTagsLineEditMode);
    d->assignNameWidget->setLayoutMode(AssignNameWidget::Compact);
    d->assignNameWidget->setAlbumModels(&d->tagModel, &d->filteredModel, &d->filterModel);
    d->assignNameWidget->lineEdit()->installEventFilter(this);
/*
    new StyleSheetDebugger(d->assignNameWidget);
*/
    return vbox;
}

void AssignNameOverlay::setActive(bool active)
{
    PersistentWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ItemInfo,QVariant)),
                this, SLOT(slotAssigned(TaggingAction,ItemInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(rejected(ItemInfo,QVariant)),
                this, SLOT(slotRejected(ItemInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(ignored(ItemInfo,QVariant)),
                this, SLOT(slotIgnored(ItemInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(ignoredClicked(ItemInfo,QVariant)),
                this, SLOT(slotUnknown(ItemInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(selected(TaggingAction,ItemInfo,QVariant)),
                this, SLOT(enterPersistentMode()));

        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ItemInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(rejected(ItemInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(ignored(ItemInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(ignoredClicked(ItemInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ItemInfo,QVariant)),
                this, SLOT(storeFocus()));

        connect(assignNameWidget(), SIGNAL(rejected(ItemInfo,QVariant)),
                this, SLOT(storeFocus()));

        connect(assignNameWidget(), SIGNAL(ignoredClicked(ItemInfo,QVariant)),
                this, SLOT(storeFocus()));

        connect(assignNameWidget(), SIGNAL(ignored(ItemInfo,QVariant)),
                this, SLOT(storeFocus()));

/*
        if (view()->model())
        {
            connect(view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
        }
*/
    }
    else
    {
        // widget is deleted

/*
        if (view() && view()->model())
        {
            disconnect(view()->model(), 0, this, 0);
        }
*/
    }
}

void AssignNameOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void AssignNameOverlay::updatePosition()
{
    if (!index().isValid())
    {
        return;
    }

    // See bug #365667.
    // Use information view below pixmap.
    // Depending of icon-view item options enabled in setup, the free space to use can be different.
    // We can continue to show the widget behind bottom of thumbnail view.

    QRect rect = itemDelegate()->imageInformationRect();
    rect.setTop(itemDelegate()->pixmapRect().top());

    if (rect.width() < m_widget->minimumSizeHint().width())
    {
        int offset = (m_widget->minimumSizeHint().width() - rect.width()) / 2;
        rect.adjust(-offset, 0, offset, 0);
    }

    int yoffset = rect.height() - m_widget->minimumSizeHint().height();
    rect.adjust(0, yoffset, 0, 0);

    QRect visualRect = m_view->visualRect(index());
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width(), m_widget->minimumSizeHint().height());
    m_widget->move(rect.topLeft());
}

void AssignNameOverlay::updateFace()
{
    if (!index().isValid() || !assignNameWidget())
    {
        return;
    }

    QVariant extraData = index().data(ItemModel::ExtraDataRole);

    /**
     * The order to plug these functions is important, since
     * setUserData() controls how the Overlay appears on
     * a particular face.
     */
    assignNameWidget()->setUserData(ItemModel::retrieveItemInfo(index()), extraData);
    assignNameWidget()->setCurrentFace(FaceTagsIface::fromVariant(extraData));
}

/*
void AssignNameOverlay::slotDataChanged(const QModelIndex& / *topLeft* /, const QModelIndex& / *bottomRight* /)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(index()))
        updateTag();
}
*/

bool AssignNameOverlay::checkIndex(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }

    QVariant extraData = index.data(ItemModel::ExtraDataRole);

    if (extraData.isNull())
    {
        return false;
    }

    return (FaceTagsIface::fromVariant(extraData).isIgnoredName() ||
            FaceTagsIface::fromVariant(extraData).isUnconfirmedType());
}

void AssignNameOverlay::showOnIndex(const QModelIndex& index)
{
    PersistentWidgetDelegateOverlay::showOnIndex(index);

/*
    // TODO: add again when fading in
    // see bug 228810, this is a small workaround

    if (m_widget && m_widget->isVisible() && index().isValid() && (index == index()))
    {
        addTagsLineEdit()->setVisibleImmediately;
    }
*/

    updatePosition();
    updateFace();
}

void AssignNameOverlay::viewportLeaveEvent(QObject* o, QEvent* e)
{
    if (isPersistent() && m_widget->isVisible())
    {
        return;
    }

    // Do not hide when hovering the pop-up of the line edit.

    if (d->isChildWidget(qApp->widgetAt(QCursor::pos()), assignNameWidget()))
    {
        return;
    }

    PersistentWidgetDelegateOverlay::viewportLeaveEvent(o, e);
}

void AssignNameOverlay::slotAssigned(const TaggingAction& action, const ItemInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    FaceTagsIface face = FaceTagsIface::fromVariant(faceIdentifier);

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Confirming" << face << action.shallAssignTag() << action.tagId();

    if (face.isConfirmedName() || !action.isValid())
    {
        return;
    }

    int tagId = 0;

    if      (action.shallAssignTag())
    {
        tagId = action.tagId();
    }
    else if (action.shallCreateNewTag())
    {
        QStringList faceNames = action.newTagName().split(QLatin1Char('/'),
                                                          Qt::SkipEmptyParts);
        if (!faceNames.isEmpty())
        {
            tagId             = -1;

            for (const QString& name : std::as_const(faceNames))
            {
                tagId = FaceTags::getOrCreateTagForPerson(name.trimmed(), tagId);
            }
        }
    }

    if (tagId)
    {
        Q_EMIT confirmFaces(affectedIndexes(index()), tagId);
    }

    hide();
}

void AssignNameOverlay::slotRejected(const ItemInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    Q_UNUSED(faceIdentifier);

    Q_EMIT removeFaces(affectedIndexes(index()));

    hide();
}

void AssignNameOverlay::slotIgnored(const ItemInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    Q_UNUSED(faceIdentifier);

    Q_EMIT ignoreFaces(affectedIndexes(index()));

    hide();
}

void AssignNameOverlay::slotUnknown(const ItemInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    Q_UNUSED(faceIdentifier);

    Q_EMIT unknownFaces(affectedIndexes(index()));

    hide();
}

void AssignNameOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(index());
}

void AssignNameOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

void AssignNameOverlay::setFocusOnWidget()
{
    if (assignNameWidget()->lineEdit())
    {
        assignNameWidget()->lineEdit()->selectAll();
        assignNameWidget()->lineEdit()->setFocus();
    }
}

bool AssignNameOverlay::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type())
    {
        case QEvent::MouseButtonPress:
        {
            enterPersistentMode();
            break;
        }

        case QEvent::FocusOut:
        {
            if (!d->isChildWidget(QApplication::focusWidget(), assignNameWidget()))
            {
                leavePersistentMode();
            }
            break;
        }

        default:
        {
            break;
        }
    }

    return PersistentWidgetDelegateOverlay::eventFilter(o, e);
}

} // namespace Digikam

#include "moc_assignnameoverlay.cpp"
