/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : Special combo box for adding or creating tags
 *
 * SPDX-FileCopyrightText: 2010       by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 1997       by Sven Radej (sven.radej@iname.com)
 * SPDX-FileCopyrightText: 1999       by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * SPDX-FileCopyrightText: 1999       by Preston Brown <pbrown at kde dot org>
 * SPDX-FileCopyrightText: 2000, 2001 by Dawit Alemayehu <adawit at kde dot org>
 * SPDX-FileCopyrightText: 2000, 2001 by Carsten Pfeiffer <pfeiffer at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "addtagscombobox.h"

// Qt includes

#include <QKeyEvent>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "tagtreeview.h"
#include "albummodel.h"

namespace Digikam
{

class Q_DECL_HIDDEN AddTagsComboBox::Private
{
public:

    Private() = default;

public:

    AddTagsLineEdit* lineEdit = nullptr;
    TaggingAction    viewTaggingAction;
};

// ---------------------------------------------------------------------------------------

AddTagsComboBox::AddTagsComboBox(QWidget* const parent)
    : TagTreeViewSelectComboBox(parent),
      d                        (new Private)
{
    setInsertPolicy(QComboBox::NoInsert); // do not let Qt interfere when Enter is pressed
    setCloseOnActivate(true);
    setMaxVisibleItems(10);
    setCheckable(false);
    setParent(nullptr);

    d->lineEdit = new AddTagsLineEdit(this);
    setLineEdit(d->lineEdit);

    // See QTBUG-20531

    Qt::WindowFlags wflags = d->lineEdit->completer()->popup()->windowFlags();
    d->lineEdit->completer()->popup()->setParent(d->lineEdit, wflags);

    connect(d->lineEdit, SIGNAL(taggingActionActivated(TaggingAction)),
            this, SLOT(slotLineEditActionActivated(TaggingAction)));

    connect(d->lineEdit, SIGNAL(taggingActionSelected(TaggingAction)),
            this, SLOT(slotLineEditActionSelected(TaggingAction)));

    connect(d->lineEdit->completer(), QOverload<const QString&>::of(&QCompleter::highlighted),
            this, [this](const QString& text)
        {
            d->lineEdit->setText(text);
        }
    );

    TagTreeView::Flags flags;
    m_treeView = new TagTreeView(this, flags);

    connect(m_treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotViewIndexActivated(QModelIndex)));

    d->lineEdit->completer()->popup()->installEventFilter(this);
    installEventFilter(this);
}

AddTagsComboBox::~AddTagsComboBox()
{
    delete d;
}

void AddTagsComboBox::setAlbumModels(TagModel* const model,
                                     TagPropertiesFilterModel* const filteredModel,
                                     CheckableAlbumFilterModel* const filterModel)
{
    TagTreeViewSelectComboBox::setAlbumModels(model, filteredModel, filterModel);

    // the line edit will pick one

    d->lineEdit->setAlbumModels(model, filteredModel, filterModel);
}

AddTagsLineEdit* AddTagsComboBox::lineEdit() const
{
    return d->lineEdit;
}

void AddTagsComboBox::setParentTag(TAlbum* const album)
{
    d->lineEdit->setParentTag(album);
}

void AddTagsComboBox::setCurrentTag(TAlbum* const album)
{
    treeView()->setCurrentAlbums(QList<Album*>() << album);
    slotViewIndexActivated(treeView()->currentIndex());
}

void AddTagsComboBox::setPlaceholderText(const QString& message)
{
    d->lineEdit->setPlaceholderText(message);
}

QString AddTagsComboBox::text() const
{
    return d->lineEdit->text();
}

void AddTagsComboBox::setText(const QString& text)
{
    d->lineEdit->setText(text);
}

TaggingAction AddTagsComboBox::currentTaggingAction()
{
    if (d->viewTaggingAction.isValid())
    {
        return d->viewTaggingAction;
    }

    return d->lineEdit->currentTaggingAction();
}

void AddTagsComboBox::slotViewIndexActivated(const QModelIndex& index)
{
    const TAlbum* const album = treeView()->albumForIndex(index);

    if (album)
    {
        d->lineEdit->setText(album->title());
        d->viewTaggingAction = TaggingAction(album->id());
    }
    else
    {
        d->lineEdit->setText(QString());
        d->viewTaggingAction = TaggingAction();
    }

    Q_EMIT taggingActionSelected(currentTaggingAction());
}

void AddTagsComboBox::slotLineEditActionActivated(const TaggingAction& action)
{
    d->viewTaggingAction = TaggingAction();

    Q_EMIT taggingActionActivated(action);
}

void AddTagsComboBox::slotLineEditActionSelected(const TaggingAction& action)
{
    d->viewTaggingAction = TaggingAction();

    Q_EMIT taggingActionSelected(action);
}

bool AddTagsComboBox::eventFilter(QObject* object, QEvent* event)
{
    QAbstractItemView* const popup = d->lineEdit->completer()->popup();

    if      (popup && (object == popup))
    {
        if      (event->type() == QEvent::Show)
        {
            setFocus();
        }
        else if (event->type() == QEvent::Hide)
        {
            popup->setFocus();
        }
        else if (event->type() == QEvent::Move)
        {
            if (parentWidget())
            {
                QPoint pos    = geometry().bottomLeft();
                QPoint newPos = parentWidget()->mapToGlobal(pos);
                newPos.ry()--;

                if (popup->pos() != newPos)
                {
                    popup->setMinimumWidth(parentWidget()->width() + 50);
                    popup->move(newPos);
                }
            }
        }

        return false;
    }
    else if (popup && (object == this))
    {
        if      ((event->type() == QEvent::KeyPress) ||
                 (event->type() == QEvent::KeyRelease))
        {
            if (popup->isVisible())
            {
                QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(event);

                if      (
                         (keyEvent->key() == Qt::Key_Up)    ||
                         (keyEvent->key() == Qt::Key_Down)  ||
                         (keyEvent->key() == Qt::Key_Enter) ||
                         (keyEvent->key() == Qt::Key_Return)
                        )
                {
                    QApplication::sendEvent(popup, event);

                    return true;
                }
                else if (keyEvent->key() == Qt::Key_Escape)
                {
                    event->accept();
                    popup->hide();

                    return true;
                }
            }
        }
        else if (event->type() == QEvent::ShortcutOverride)
        {
            QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(event);

            if      (popup->isVisible())
            {
                if (
                    (keyEvent->key() == Qt::Key_Up)   ||
                    (keyEvent->key() == Qt::Key_Down) ||
                    (keyEvent->key() == Qt::Key_Escape)
                   )
                {
                    event->accept();
                }
            }
            else if (d->lineEdit->text().isEmpty())
            {
                if (
                    (keyEvent->key() == Qt::Key_Left) ||
                    (keyEvent->key() == Qt::Key_Right)
                   )
                {
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::HoverMove)
        {
            if (popup->isVisible())
            {
                popup->hide();
            }
        }
        else if (event->type() == QEvent::Show)
        {
            adjustSize();
        }

        return false;
    }

    return TagTreeViewSelectComboBox::eventFilter(object, event);
}

} // namespace Digikam

#include "moc_addtagscombobox.cpp"
