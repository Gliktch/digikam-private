/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-03
 * Description : an input widget for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPlainTextEdit>
#include <QWidget>
#include <QComboBox>

// Local includes

#include "comboboxutilities.h"
#include "parser.h"

class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QEvent;

namespace Digikam
{

class AdvancedRenameLineEdit : public QPlainTextEdit
{
    Q_OBJECT

public:

    explicit AdvancedRenameLineEdit(QWidget* const parent = nullptr);
    ~AdvancedRenameLineEdit()                     override;

    void    setParser(Parser* parser);
    Parser* parser()                        const;

    void setParseTimerDuration(int milliseconds);

public Q_SLOTS:

    void slotSetText(const QString&);
    void slotCursorPositionChanged();

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

protected:

    void keyPressEvent(QKeyEvent* e)              override;
    void wheelEvent(QWheelEvent* e)               override;
    void scrollContentsBy(int dx, int dy)         override;

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();

private:

    void setupWidgets();
    void setupConnections();

private:

    AdvancedRenameLineEdit(const AdvancedRenameLineEdit&)            = delete;
    AdvancedRenameLineEdit& operator=(const AdvancedRenameLineEdit&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------

class AdvancedRenameInput : public QComboBox
{
    Q_OBJECT

public:

    explicit AdvancedRenameInput(QWidget* const parent = nullptr);
    ~AdvancedRenameInput()                            override;

    void setParser(Parser* parser);
    void setParseTimerDuration(int milliseconds);

    QString text()                              const;
    void    setText(const QString& text);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

public Q_SLOTS:

    void slotAddToken(const QString&);
    void slotClearText();
    void slotClearTextAndHistory();
    void slotSetFocus();

    void slotHighlightLineEdit();
    void slotHighlightLineEdit(const QString& word);

protected:

    void changeEvent(QEvent* e)                       override;

private Q_SLOTS:

    void slotClearButtonPressed();
    void slotTextChanged(const QString& text);
    void slotIndexChanged(int index);

private:

    void readSettings();
    void writeSettings();
    void enableHighlighter(bool enable);

    void setupWidgets();
    void setupConnections();

private:

    /// @note disabled
    AdvancedRenameInput(const AdvancedRenameInput&)            = delete;
    AdvancedRenameInput& operator=(const AdvancedRenameInput&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
