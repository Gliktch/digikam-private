/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : A widget to host settings as expander box
 *
 * SPDX-FileCopyrightText: 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Manuel Viet <contact at 13zenrv dot fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QScrollArea>
#include <QFrame>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

/**
 * @brief A widget to show an horizontal or vertical line separator
 */
class DIGIKAM_EXPORT DLineWidget : public QFrame
{
    Q_OBJECT

public:

    explicit DLineWidget(Qt::Orientation orientation, QWidget* const parent = nullptr);
    ~DLineWidget() override;
};

// -------------------------------------------------------------------------

/**
 * @brief A label to show text adjusted to widget size
 */
class DIGIKAM_EXPORT DAdjustableLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DAdjustableLabel(QWidget* const parent = nullptr);
    ~DAdjustableLabel() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint()        const override;

    void setAlignment(Qt::Alignment align);
    void setElideMode(Qt::TextElideMode mode);

    QString adjustedText()  const;

public Q_SLOTS:

    void setAdjustedText(const QString& text = QString());

private:

    void resizeEvent(QResizeEvent*) override;
    void adjustTextToLabel();

    /// @note disabledd methods from QLabel
    QString text() const { return QString(); }; ///< Use adjustedText() instead.
    void setText(const QString&) {};            ///< Use setAdjustedText(text) instead.
    void clear()                 {};            ///< Use setdjustedText(QString()) instead.

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DClickLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DClickLabel(QWidget* const parent = nullptr);
    explicit DClickLabel(const QString& text, QWidget* const parent = nullptr);
    ~DClickLabel()                             override;

Q_SIGNALS:

    /// @brief Emitted when activated by left mouse click
    void leftClicked();

    /// @brief Emitted when activated, by mouse or key press
    void activated();

protected:

    void mousePressEvent(QMouseEvent* event)   override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event)       override;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DSqueezedClickLabel : public DAdjustableLabel
{
    Q_OBJECT

public:

    explicit DSqueezedClickLabel(QWidget* const parent = nullptr);
    explicit DSqueezedClickLabel(const QString& text, QWidget* const parent = nullptr);
    ~DSqueezedClickLabel()                     override;

Q_SIGNALS:

    void leftClicked();
    void activated();

protected:

    void mousePressEvent(QMouseEvent* event)   override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event)       override;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    explicit DArrowClickLabel(QWidget* const parent = nullptr);
    ~DArrowClickLabel()                               override = default;

    void setArrowType(Qt::ArrowType arrowType);
    Qt::ArrowType arrowType()                   const;

    QSize sizeHint ()                           const override;

Q_SIGNALS:

    void leftClicked();

protected:

    void mousePressEvent(QMouseEvent* event)          override;
    void mouseReleaseEvent(QMouseEvent* event)        override;
    void paintEvent(QPaintEvent* event)               override;

protected:

    Qt::ArrowType m_arrowType   = Qt::DownArrow;
    int           m_size        = 8;
    int           m_margin      = 2;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DLabelExpander : public QWidget
{
    Q_OBJECT

public:

    explicit DLabelExpander(QWidget* const parent = nullptr);
    ~DLabelExpander()                 override;

    void setCheckBoxVisible(bool b);
    bool checkBoxIsVisible()    const;

    void setChecked(bool b);
    bool isChecked()            const;

    void setLineVisible(bool b);
    bool lineIsVisible()        const;

    void setText(const QString& txt);
    QString text()              const;

    void setButtonVisible(bool b);
    bool buttonIsVisible()      const;
    void setButtonIcon(const QIcon& icon);
    void setButtonToolTip(const QString& tip);

    void setIcon(const QIcon &icon);
    QIcon icon()                const;

    void setWidget(QWidget* const widget);
    QWidget* widget()           const;

    void setExpanded(bool b);
    bool isExpanded()           const;

    void setExpandByDefault(bool b);
    bool isExpandByDefault()    const;

Q_SIGNALS:

    void signalExpanded(bool);
    void signalToggled(bool);
    void signalButtonPressed();

private Q_SLOTS:

    void slotToggleContainer();
    void slotUpdateIcon();

private:

    bool eventFilter(QObject* obj,
                     QEvent* ev)      override;

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DExpanderBox : public QScrollArea
{
    Q_OBJECT

public:

    explicit DExpanderBox(QWidget* const parent = nullptr);
    ~DExpanderBox()                                   override;

    /**
     * @brief Add DLabelExpander item at end of box layout with these settings:
     * @arg w               : the widget hosted by DLabelExpander.
     * @arg pix             : pixmap used as icon to item title.
     * @arg txt             : text used as item title.
     * @arg objName         : item object name used to read/save expanded settings to rc file.
     * @arg expandBydefault : item state by default (expanded or not).
     */
    void addItem(QWidget* const w, const QIcon &icon, const QString& txt,
                 const QString& objName, bool expandBydefault);
    void addItem(QWidget* const w, const QString& txt,
                 const QString& objName, bool expandBydefault);

    /**
     * @brief Insert DLabelExpander item at box layout index with these settings:
     * @arg w               : the widget hosted by DLabelExpander.
     * @arg pix             : pixmap used as icon to item title.
     * @arg txt             : text used as item title.
     * @arg objName         : item object name used to read/save expanded settings to rc file.
     * @arg expandBydefault : item state by default (expanded or not).
     */
    void insertItem(int index, QWidget* const w, const QIcon &icon, const QString& txt,
                    const QString& objName, bool expandBydefault);
    void insertItem(int index, QWidget* const w, const QString& txt,
                    const QString& objName, bool expandBydefault);

    void removeItem(int index);

    void setCheckBoxVisible(int index, bool b);
    bool checkBoxIsVisible(int index)           const;

    void setChecked(int index, bool b);
    bool isChecked(int index)                   const;

    void setItemText(int index, const QString& txt);
    QString itemText (int index)                const;

    void setItemIcon(int index, const QIcon &icon);
    QIcon itemIcon(int index)                   const;

    void setButtonVisible(int index, bool b);
    bool buttonIsVisible(int index)             const;
    void setButtonIcon(int index, const QIcon& icon);
    void setButtonToolTip(int index, const QString& tip);

    void setItemToolTip(int index, const QString& tip);
    QString itemToolTip(int index)              const;

    void setItemEnabled(int index, bool enabled);
    bool isItemEnabled(int index)               const;

    void addStretch();
    void insertStretch(int index);

    void setItemExpanded(int index, bool b);
    bool isItemExpanded(int index)              const;

    int  count() const;

    DLabelExpander* widget(int index)           const;
    int indexOf(DLabelExpander* const widget)   const;

    virtual void readSettings(KConfigGroup& group);
    virtual void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalItemExpanded(int index, bool b);
    void signalItemToggled(int index, bool b);
    void signalItemButtonPressed(int index);

private Q_SLOTS:

    void slotItemExpanded(bool b);
    void slotItemToggled(bool b);
    void slotItemButtonPressed();

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DExpanderBoxExclusive : public DExpanderBox
{
    Q_OBJECT

public:

    explicit DExpanderBoxExclusive(QWidget* const parent = nullptr);
    ~DExpanderBoxExclusive()   override = default;

    /**
     * @brief Show one expander open at most
     */
    void setIsToolBox(bool b);
    bool isToolBox()    const;

private Q_SLOTS:

    void slotItemExpanded(bool b);

private:

    bool m_toolbox  = true;
};

} // namespace Digikam
