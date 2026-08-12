/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "rule.h"
#include "digikam_export.h"

class QMenu;

namespace Digikam
{

class ParseSettings;
class Parser;

class DIGIKAM_GUI_EXPORT AdvancedRenameWidget : public QWidget
{
    Q_OBJECT

public:

    enum LayoutStyle
    {
        LayoutNormal,
        LayoutCompact
    };

    enum ControlWidget
    {
        None               = 0x0,
        ToolTipButton      = 0x1,
        TokenButtons       = 0x2,
        ModifierToolButton = 0x4,
        DefaultControls    = TokenButtons | ToolTipButton | ModifierToolButton
    };
    Q_DECLARE_FLAGS(ControlWidgets, ControlWidget)

public:

    explicit AdvancedRenameWidget(QWidget* const parent = nullptr);
    ~AdvancedRenameWidget() override;

    /**
     * returns the current parse string
     */
    QString parseString() const;

    /**
     *
     * sets the current parse string
     * @param text the new parse string
     */
    void setParseString(const QString& text);

    void setParseTimerDuration(int milliseconds);

    /**
     * resets the current parse string, the LineEdit widget will be empty
     */
    void clearParseString();

    /**
     * clears the parse string as well as the history
     */
    void clear();

    /**
     * sets the current parser. If a parser has already been assigned, it will
     * be deleted first.
     * @param parser a pointer to the new parser instance
     */
    void setParser(Parser* parser);

    /**
     * returns a pointer to the currently assigned parser
     */
    Parser* parser() const;

    /**
     * evaluates the parse string and executes the parser
     * @param settings information about the file to be renamed
     * @return the new name of the file
     */
    QString parse(ParseSettings& settings) const;

    /**
     * sets the layout of the control widgets
     * @see ControlWidget
     * @param mask a bitmask for setting the control widgets
     */
    void setControlWidgets(ControlWidgets mask);

    /**
     * set the layout style of the widget
     * @param style the style of the layout
     * @see LayoutStyle
     */
    void setLayoutStyle(LayoutStyle style);

    /**
     * set focus for the LineEdit widget
     */
    void focusLineEdit();

    /**
     * highlight the LineEdit widgets text
     */
    void highlightLineEdit();

    /**
     * highlight a word in the LineEdit widgets text
     */
    void highlightLineEdit(const QString& word);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalReturnPressed();

private Q_SLOTS:

    void slotToolTipButtonToggled(bool);
    void slotTokenMarked(bool);

private:

    void createToolTip();

    void   registerParserControls();
    QMenu* createControlsMenu(QWidget* const parent, const RulesList& rules);

    void calculateLayout();

    void setupWidgets();

    void readSettings();
    void writeSettings();

private:

    /// @note disabled
    AdvancedRenameWidget(const AdvancedRenameWidget&)            = delete;
    AdvancedRenameWidget& operator=(const AdvancedRenameWidget&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::AdvancedRenameWidget::ControlWidgets)
