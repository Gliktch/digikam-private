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

#include "advancedrenameinput.h"

// Qt includes

#include <QFontMetrics>
#include <QLayout>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "highlighter.h"

namespace Digikam
{

class Q_DECL_HIDDEN AdvancedRenameLineEdit::Private
{
public:

    Private() = default;

public:

    const quint8 INVALID            = -1;
    const QString DUMMY_TEXT        = QLatin1String("DUMMY_TEXT_y_fjqp|");

public:

    bool    allowDirectoryCreation  = false;
    int     verticalSliderPosition  = INVALID;

    QString lastPlainText;

    QTimer* parseTimer              = nullptr;
    Parser* parser                  = nullptr;
};

AdvancedRenameLineEdit::AdvancedRenameLineEdit(QWidget* const parent)
    : QPlainTextEdit(parent),
      d             (new Private)
{
    setupWidgets();
    setupConnections();
}

AdvancedRenameLineEdit::~AdvancedRenameLineEdit()
{
    delete d;
}

void AdvancedRenameLineEdit::setupWidgets()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setFocusPolicy(Qt::StrongFocus);
    setFrameStyle(QFrame::NoFrame);
    setPalette(qApp->palette());

    QFontMetrics fm = fontMetrics();
    int textHeight  = fm.boundingRect(d->DUMMY_TEXT).height();
    document()->setDocumentMargin(1);
    setFixedHeight(textHeight + 8);

    // --------------------------------------------------------

    d->parseTimer = new QTimer(this);
    d->parseTimer->setInterval(500);
    d->parseTimer->setSingleShot(true);

    // --------------------------------------------------------

    // layout widget correctly by setting a dummy text and calling ensureCursorVisible().
    // Save the scrollbar position now, to avoid scrolling of the text when selecting with the mouse

    setPlainText(d->DUMMY_TEXT);
    ensureCursorVisible();
    d->verticalSliderPosition = verticalScrollBar()->value();
    clear();
}

void AdvancedRenameLineEdit::setupConnections()
{
    connect(d->parseTimer, SIGNAL(timeout()),
            this, SLOT(slotParseTimer()));

    connect(this, SIGNAL(textChanged()),
            this, SLOT(slotTextChanged()));

    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));
}

void AdvancedRenameLineEdit::setParseTimerDuration(int milliseconds)
{
    d->parseTimer->setInterval(milliseconds);
}

void AdvancedRenameLineEdit::setParser(Parser* parser)
{
    if (parser)
    {
        d->parser = parser;
    }
}

Parser* AdvancedRenameLineEdit::parser() const
{
    return d->parser;
}

void AdvancedRenameLineEdit::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        // avoid newlines in the new name

        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            slotParseTimer();

            Q_EMIT signalReturnPressed();

            break;
        }

        // the keys "Up, Down, PageUp, PageDown" should be send to the QComboBox

        case Qt::Key_Up:
        case Qt::Key_PageUp:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        {
            e->setAccepted(false);
            break;
        }

        // the key "/" should not be allowed (QTextEdit is not able to use a QValidator, so we must do it in here)

        case Qt::Key_Slash:
        {
            if (!d->allowDirectoryCreation)
            {
                // do nothing
            }

            break;
        }

        default:
        {
            QPlainTextEdit::keyPressEvent(e);
            break;
        }
    }
}

void AdvancedRenameLineEdit::wheelEvent(QWheelEvent* e)
{
    e->setAccepted(false);
}

void AdvancedRenameLineEdit::slotTextChanged()
{
    d->parseTimer->start();
}

void AdvancedRenameLineEdit::slotParseTimer()
{
    if (d->lastPlainText != toPlainText())
    {
        d->lastPlainText = toPlainText();

        Q_EMIT signalTextChanged(d->lastPlainText);
    }
}

void AdvancedRenameLineEdit::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)

    if (d->verticalSliderPosition != d->INVALID)
    {
        verticalScrollBar()->setValue(d->verticalSliderPosition);
    }

    viewport()->update();
}

void AdvancedRenameLineEdit::slotCursorPositionChanged()
{
    bool found = false;

    if (d->parser)
    {
        ParseSettings settings;
        settings.parseString = toPlainText();
        int start            = d->INVALID;
        int length           = d->INVALID;
        int pos              = textCursor().position();
        found                = d->parser->tokenAtPosition(settings, pos, start, length);
        found                = found && ((start + length) == pos);
    }

    Q_EMIT signalTokenMarked(found);
}

void AdvancedRenameLineEdit::slotSetText(const QString& text)
{
    clear();
    setPlainText(text);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cursor);
    setFocus();
}

// --------------------------------------------------------

class Q_DECL_HIDDEN AdvancedRenameInput::Private
{
public:

    Private() = default;

public:

    const QString configGroupName               = QLatin1String("AdvancedRename Input");
    const QString configPatternHistoryListEntry = QLatin1String("Pattern History List");

    const int               maxVisibleItems     = 10;
    const int               maxHistoryItems     = 30;

public:

    AdvancedRenameLineEdit* lineEdit            = nullptr;
    ProxyLineEdit*          proxy               = nullptr;
    Highlighter*            highlighter         = nullptr;
};

// --------------------------------------------------------

AdvancedRenameInput::AdvancedRenameInput(QWidget* const parent)
    : QComboBox(parent),
      d        (new Private)
{
    setupWidgets();
    setupConnections();

    readSettings();
}

AdvancedRenameInput::~AdvancedRenameInput()
{
    writeSettings();
    delete d;
}

void AdvancedRenameInput::setParser(Parser* parser)
{
    d->lineEdit->setParser(parser);
    enableHighlighter(true);
}

void AdvancedRenameInput::setParseTimerDuration(int milliseconds)
{
    d->lineEdit->setParseTimerDuration(milliseconds);
}

void AdvancedRenameInput::setText(const QString& text)
{
    d->lineEdit->slotSetText(text);
}

void AdvancedRenameInput::slotClearText()
{
    d->lineEdit->clear();
}

void AdvancedRenameInput::slotClearTextAndHistory()
{
    d->lineEdit->clear();
    clear();
}

void AdvancedRenameInput::slotSetFocus()
{
    d->lineEdit->setFocus();
    d->lineEdit->ensureCursorVisible();
}

void AdvancedRenameInput::slotHighlightLineEdit()
{
    d->lineEdit->selectAll();
}

void AdvancedRenameInput::slotHighlightLineEdit(const QString& word)
{
    QTextCursor cursor = d->lineEdit->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    d->lineEdit->setTextCursor(cursor);
    d->lineEdit->find(word, QTextDocument::FindCaseSensitively);
}

void AdvancedRenameInput::enableHighlighter(bool enable)
{
    delete d->highlighter;
    d->highlighter = enable ? new Highlighter(d->lineEdit->document(), d->lineEdit->parser())
                            : nullptr;
}

void AdvancedRenameInput::setupWidgets()
{
    setEditable(true);

    setMaxVisibleItems(d->maxVisibleItems);
    setMaxCount(d->maxHistoryItems);

    d->lineEdit = new AdvancedRenameLineEdit(this);
    d->proxy    = new ProxyLineEdit(this);
    d->proxy->setWidget(d->lineEdit);
    d->proxy->setClearButtonShown(true);
    d->proxy->setContentsMargins(0, -2, 0, 2);

    setMinimumHeight(d->lineEdit->height() + 2);
    setLineEdit(d->proxy);
}

void AdvancedRenameInput::setupConnections()
{
    connect(d->proxy, SIGNAL(signalClearButtonPressed()),
            this, SLOT(slotClearButtonPressed()));

    connect(d->lineEdit, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    connect(d->lineEdit, SIGNAL(signalTokenMarked(bool)),
            this, SIGNAL(signalTokenMarked(bool)));

    connect(d->lineEdit, SIGNAL(signalReturnPressed()),
            this, SIGNAL(signalReturnPressed()));

    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotIndexChanged(int)));
}

void AdvancedRenameInput::slotIndexChanged(int index)
{
    setText(itemText(index));
}

void AdvancedRenameInput::changeEvent(QEvent* e)
{
    QComboBox::changeEvent(e);

    if (e->type() == QEvent::EnabledChange)
    {
        enableHighlighter(isEnabled());
    }
}

void AdvancedRenameInput::slotClearButtonPressed()
{
    slotClearText();
    slotSetFocus();
}

void AdvancedRenameInput::slotTextChanged(const QString& text)
{
    d->proxy->setText(text);

    Q_EMIT signalTextChanged(text);
}

QString AdvancedRenameInput::text() const
{
    return d->lineEdit->toPlainText();
}

void AdvancedRenameInput::slotAddToken(const QString& token)
{
    QString tokenStr = token;
    tokenStr.remove(QLatin1String("||"));
    d->lineEdit->insertPlainText(tokenStr);
    slotSetFocus();
}

void AdvancedRenameInput::readSettings()
{
    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(d->configGroupName);
    QStringList patternHistory = group.readEntry(d->configPatternHistoryListEntry, QStringList());
    patternHistory.removeAll(QLatin1String(""));
    patternHistory.removeDuplicates();
    addItems(patternHistory);
    d->lineEdit->clear();
    setCurrentIndex(-1);
}

void AdvancedRenameInput::writeSettings()
{
    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(d->configGroupName);
    QStringList patternHistory = group.readEntry(d->configPatternHistoryListEntry, QStringList());

    // remove duplicate entries and save pattern history, omit empty strings

    QString pattern = d->lineEdit->toPlainText();
    patternHistory.removeAll(QLatin1String(""));
    patternHistory.removeAll(pattern);
    patternHistory.removeDuplicates();
    patternHistory.prepend(pattern);

    QStringList writeHistory;

    for (int i = 0 ; i < qMin(patternHistory.size(), d->maxHistoryItems) ; ++i)
    {
        writeHistory << patternHistory.at(i);
    }

    group.writeEntry(d->configPatternHistoryListEntry, writeHistory);
}

} // namespace Digikam

#include "moc_advancedrenameinput.cpp"
