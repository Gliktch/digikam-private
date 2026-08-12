/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QLabel>
#include <QListView>
#include <QComboBox>
#include <QPersistentModelIndex>
#include <QLineEdit>

// Local includes

#include "digikam_export.h"

class QVBoxLayout;
class QTreeView;

namespace Digikam
{

class DIGIKAM_EXPORT ProxyLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    /**
     * This class will not act as a QLineEdit at all,
     * but present another widget (any kind of widget)
     * instead in the space assigned to the QLineEdit.
     * Use this class if you need to pass a QLineEdit but
     * want actually to use a different widget.
     */
    explicit ProxyLineEdit(QWidget* const parent = nullptr);

    /**
     * After constructing, set the actual widget here
     */
    virtual void setWidget(QWidget* widget);

    void setClearButtonShown(bool show);

Q_SIGNALS:

    void signalClearButtonPressed();

private Q_SLOTS:

    void slotTextChanged(const QString& text);

protected:

    QSize minimumSizeHint() const                   override;
    QSize sizeHint()        const                   override;

    void mousePressEvent(QMouseEvent* event)        override;
    void mouseMoveEvent(QMouseEvent* event)         override;
    void mouseReleaseEvent(QMouseEvent* event)      override;
    void mouseDoubleClickEvent(QMouseEvent* event)  override;
    void keyPressEvent(QKeyEvent* event)            override;
    void focusInEvent(QFocusEvent* event)           override;
    void focusOutEvent(QFocusEvent* event)          override;
    void paintEvent(QPaintEvent* event)             override;
    void dragEnterEvent(QDragEnterEvent* event)     override;
    void dragMoveEvent(QDragMoveEvent* e)           override;
    void dragLeaveEvent(QDragLeaveEvent* e)         override;
    void dropEvent(QDropEvent* event)               override;
    void changeEvent(QEvent* event)                 override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void inputMethodEvent(QInputMethodEvent* event) override;

protected:

    QWidget*     m_widget = nullptr;
    QVBoxLayout* m_layout = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ProxyClickLineEdit : public ProxyLineEdit
{
    Q_OBJECT

public:

    /**
     * A ProxyLineEdit that emits leftClicked() on
     * mouse press event.
     * Press on the held widget will result in the signal
     * if the widget does not accept() them.
     */
    explicit ProxyClickLineEdit(QWidget* const parent = nullptr);

Q_SIGNALS:

    void leftClicked();

protected:

    void mouseReleaseEvent(QMouseEvent* event) override;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ModelIndexBasedComboBox : public QComboBox
{
    Q_OBJECT

public:

    /**
     * QComboBox has a current index based on a single integer.
     * This is not sufficient for more complex models.
     * This class is a combo box that stores a current index
     * based on QModelIndex.
     */
    explicit ModelIndexBasedComboBox(QWidget* const  parent = nullptr);

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& index);

    void hidePopup() override;
    void showPopup() override;

protected:

    QPersistentModelIndex m_currentIndex;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT StayPoppedUpComboBox : public ModelIndexBasedComboBox
{
    Q_OBJECT

public:

    /**
     * This class provides an abstract QComboBox with a custom view
     * (which is created by implementing subclasses)
     * instead of the usual QListView.
     * The Pop-up of the combo box will stay open after selecting an item;
     * it will be closed by clicking outside, but not inside the widget.
     * You need three steps:
     * Construct the object, call setModel() with an appropriate
     * QAbstractItemModel, then call installView() to replace
     * the standard combo box view with a view.
     */
    explicit StayPoppedUpComboBox(QWidget* const parent = nullptr);

protected:

    /**
     * Replace the standard combo box list view with the given view.
     * The view will be set as the view of the combo box
     * (including re-parenting) and be stored in the m_view variable.
     */
    void installView(QAbstractItemView* view);

    /**
     * Implement in subclass:
     * Send the given event to the viewportEvent() method of m_view.
     * This method is protected for a usual QAbstractItemView.
     * You can override, pass a view, and call parent implementation.
     * The existing view will be used. You must then also
     * reimplement sendViewportEventToView.
     */
    virtual void sendViewportEventToView(QEvent* e) = 0;

    bool eventFilter(QObject* watched, QEvent* event) override;

protected:

    QAbstractItemView* m_view = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT TreeViewComboBox : public StayPoppedUpComboBox
{
    Q_OBJECT

public:

    /**
     * This class provides a QComboBox with a QTreeView
     * instead of the usual QListView.
     * You need three steps:
     * Construct the object, call setModel() with an appropriate
     * QAbstractItemModel, then call installView() to replace
     * the standard combo box view with a QTreeView.
     */
    explicit TreeViewComboBox(QWidget* parent = nullptr);

    /**
     * Replace the standard combo box list view with a QTreeView.
     * Call this after installing an appropriate model.
     */
    virtual void installView(QAbstractItemView* view = nullptr);

    /**
     * Returns the QTreeView of this class. Valid after installView() has been called
     */
    virtual QTreeView* view() const;

protected:

    void sendViewportEventToView(QEvent* e) override;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ListViewComboBox : public StayPoppedUpComboBox
{
    Q_OBJECT

public:

    /**
     * This class provides an implementation of a StayPoppedUpComboBox
     * with a QListView. This is the standard view of a QComboBox,
     * but in conjunction with StayPoppedUpComboBox some extra steps are needed.
     * You need three steps:
     * Construct the object, call setModel() with an appropriate
     * QAbstractItemModel, then call installView().
     */
    explicit ListViewComboBox(QWidget* parent = nullptr);

    /**
     * Returns the QTreeView of this class. Valid after installView() has been called.
     */
    QListView* view() const;

    /**
     * Replace the standard combo box list view with a QTreeView.
     * Call this after installing an appropriate model.
     */
    virtual void installView(QAbstractItemView* view = nullptr);

protected:

    void sendViewportEventToView(QEvent* e) override;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT TreeViewLineEditComboBox : public TreeViewComboBox
{
    Q_OBJECT

public:

    /**
     * This class provides a TreeViewComboBox
     * with a read-only line edit.
     * The text in the line edit can be adjusted. The combo box will
     * open on a click on the line edit.
     * You need three steps:
     * Construct the object, call setModel() with an appropriate
     * QAbstractItemModel, then call installView() to replace
     * the standard combo box view with a QTreeView.
     */
    explicit TreeViewLineEditComboBox(QWidget* const parent = nullptr);

    /**
     * Set the text of the line edit (the text that is visible
     * if the popup is not opened).
     * Applicable only for default installLineEdit() implementation.
     */
    void setLineEditText(const QString& text);

    void setLineEdit(QLineEdit* edit);

    /**
     * Replace the standard combo box list view with a QTreeView.
     * Call this after installing an appropriate model.
     */
    void installView(QAbstractItemView* view = nullptr) override;

protected:

    /**
     * Sets a line edit. Called by installView().
     * The default implementation is described above.
     * An empty implementation will keep the default QComboBox line edit.
     */
    virtual void installLineEdit();

protected:

    QLineEdit* m_comboLineEdit = nullptr;
};

} // namespace Digikam
