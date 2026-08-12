/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-01
 * Description : Text edit widgets with spellcheck support and edition limitations.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dtextedit_p.h"

namespace Digikam
{

DPlainTextEdit::DPlainTextEdit(QWidget* const parent)
    : QPlainTextEdit(parent),
      d             (new Private)
{
    d->init(this);
}

DPlainTextEdit::DPlainTextEdit(const QString& contents, QWidget* const parent)
    : QPlainTextEdit(parent),
      d             (new Private)
{
    d->init(this);
    setPlainText(contents);
}

DPlainTextEdit::DPlainTextEdit(unsigned int lines, QWidget* const parent)
    : QPlainTextEdit(parent),
      d             (new Private)
{
    d->lines = lines;
    d->init(this);
}

DPlainTextEdit::~DPlainTextEdit()
{

#ifdef HAVE_SONNET

    delete d->spellChecker;

#endif

    delete d;
}

bool DPlainTextEdit::isClearButtonEnabled() const
{
    return d->clearBtnEnable;
}

void DPlainTextEdit::setClearButtonEnabled(bool enable)
{
    d->clearBtnEnable = enable;
}

void DPlainTextEdit::setLinesVisible(unsigned int lines)
{
    if (lines == 0)
    {
        return;
    }

    d->lines = lines;

    // Mimic QLineEdit

    if (d->lines == 1)
    {
        setLineWrapMode(QPlainTextEdit::NoWrap);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        verticalScrollBar()->setFixedHeight(0);
    }
}

unsigned int DPlainTextEdit::linesVisible() const
{
    return d->lines;
}

QString DPlainTextEdit::text() const
{
    return toPlainText();
}

void DPlainTextEdit::setText(const QString& text)
{
    QString maskedTxt = text;

    for (int i = 0 ; i < d->ignoredMask.size() ; ++i)
    {
        maskedTxt.remove(d->ignoredMask[i]);
    }

    if (!d->acceptedMask.isEmpty())
    {
        QString maskedTxt2;

        for (int i = 0 ; i < maskedTxt.size() ; ++i)
        {
            if (d->acceptedMask.contains(maskedTxt[i]))
            {
                maskedTxt2.append(maskedTxt[i]);
            }
        }

        maskedTxt = maskedTxt2;
    }

    setPlainText(maskedTxt);
}

QString DPlainTextEdit::ignoredCharacters() const
{
    return d->ignoredMask;
}

void DPlainTextEdit::setIgnoredCharacters(const QString& mask)
{
    d->ignoredMask = mask;
}

QString DPlainTextEdit::acceptedCharacters() const
{
    return d->acceptedMask;
}

void DPlainTextEdit::setAcceptedCharacters(const QString& mask)
{
    d->acceptedMask = mask;
}

void DPlainTextEdit::setCurrentLanguage(const QString& lang)
{

#ifdef HAVE_SONNET

    if (!lang.isEmpty())
    {
        d->spellChecker->highlighter()->setAutoDetectLanguageDisabled(true);
        d->spellChecker->highlighter()->setCurrentLanguage(lang);

        qCDebug(DIGIKAM_WIDGETS_LOG) << "Spell Checker Language:" << currentLanguage();

        d->spellChecker->highlighter()->rehighlight();
    }
    else if (!d->container.defaultLanguage.isEmpty())
    {
        d->spellChecker->highlighter()->setAutoDetectLanguageDisabled(true);
        d->spellChecker->highlighter()->setCurrentLanguage(d->container.defaultLanguage);

        qCDebug(DIGIKAM_WIDGETS_LOG) << "Spell Checker Language:" << currentLanguage();

        d->spellChecker->highlighter()->rehighlight();
    }
    else
    {
        d->spellChecker->highlighter()->setAutoDetectLanguageDisabled(false);

        qCDebug(DIGIKAM_WIDGETS_LOG) << "Spell Checker Language auto-detection enabled";

        d->spellChecker->highlighter()->rehighlight();
    }

#else

    Q_UNUSED(lang);

#endif

}

QString DPlainTextEdit::currentLanguage() const
{

#ifdef HAVE_SONNET

    return d->spellChecker->highlighter()->currentLanguage();

#else

    return QString();

#endif

}

void DPlainTextEdit::keyPressEvent(QKeyEvent* e)
{
    if ((d->maxLength > 0) && (text().length() >= d->maxLength))
    {
        QString txt       = e->text();
        int key           = e->key();
        bool delCondition = (key == Qt::Key_Delete)    ||
                            (key == Qt::Key_Backspace) ||
                            (key == Qt::Key_Cancel);

        if (txt.isEmpty() || delCondition)
        {
            QPlainTextEdit::keyPressEvent(e);

            return;
        }
    }

    if (d->lines == 1)
    {
        int key = e->key();

        if ((key == Qt::Key_Return) || (key == Qt::Key_Enter))
        {
            e->ignore();

            Q_EMIT returnPressed();

            return;
        }

        for (int i = 0 ; i < d->ignoredMask.size() ; ++i)
        {
            if (key == d->ignoredMask[i].unicode())
            {
                e->ignore();
                return;
            }
        }

        for (int i = 0 ; i < d->acceptedMask.size() ; ++i)
        {
            if (key != d->acceptedMask[i].unicode())
            {
                e->ignore();
                return;
            }
        }

        Q_EMIT textEdited(text());
    }

    QPlainTextEdit::keyPressEvent(e);
}

void DPlainTextEdit::insertFromMimeData(const QMimeData* source)
{
    QMimeData scopy;

    if (source->hasHtml())
    {
        scopy.setHtml(source->html());
    }

    if (source->hasText())
    {
        scopy.setText(source->text());
    }

    if (source->hasUrls())
    {
        scopy.setUrls(source->urls());
    }

    if ((d->lines == 1) && source->hasText())
    {
        QString textToPaste = source->text();
        textToPaste.replace(QLatin1String("\n\r"), QLatin1String(" "));
        textToPaste.replace(QLatin1Char('\n'),     QLatin1Char(' '));
        scopy.setText(textToPaste);
    }

    QString maskedTxt = scopy.text();

    for (int i = 0 ; i < d->ignoredMask.size() ; ++i)
    {
        maskedTxt.remove(d->ignoredMask[i]);
    }

    scopy.setText(maskedTxt);

    if (!d->acceptedMask.isEmpty())
    {
        QString maskedTxt2;

        for (int i = 0 ; i < maskedTxt.size() ; ++i)
        {
            if (d->acceptedMask.contains(maskedTxt[i]))
            {
                maskedTxt2.append(maskedTxt[i]);
            }
        }

        scopy.setText(maskedTxt2);
    }

    if ((d->maxLength > 0) && source->hasText())
    {
        QString textToPaste = scopy.text();
        QString curText     = text();
        int totalLength     = curText.length() + textToPaste.length();

        if      (curText.length() == d->maxLength)
        {
            scopy.setText(QString());
        }
        else if (totalLength > d->maxLength)
        {
            int numToDelete = totalLength - d->maxLength;
            textToPaste     = textToPaste.left(textToPaste.length() - numToDelete);
            scopy.setText(textToPaste);
        }
    }

    QPlainTextEdit::insertFromMimeData(&scopy);
}

void DPlainTextEdit::setLocalizeSettings(const LocalizeContainer& settings)
{
    d->container = settings;

#ifdef HAVE_SONNET

    // Automatic disable spellcheck if too many spelling errors are detected.

    d->spellChecker->highlighter()->setAutomatic(!d->container.enableSpellCheck);

    // Enable spellchecker globally.

    d->spellChecker->highlighter()->setActive(d->container.enableSpellCheck);

    for (const QString& word : std::as_const(d->container.ignoredWords))
    {
        d->spellChecker->highlighter()->ignoreWord(word);
    }

    d->spellChecker->highlighter()->rehighlight();

#endif

}

LocalizeContainer DPlainTextEdit::spellCheckSettings() const
{
    return d->container;
}

int DPlainTextEdit::maxLength() const
{
    return d->maxLength;
}

int DPlainTextEdit::leftCharacters() const
{
    int left = (d->maxLength - toPlainText().length());

    return ((left > 0) ? left : 0);
}

void DPlainTextEdit::setMaxLength(int length)
{
    d->maxLength    = length;
    QString curText = text();

    if ((d->maxLength > 0) && (curText.length() > d->maxLength))
    {
        curText = curText.left(d->maxLength);
        setText(curText);
    }
}

void DPlainTextEdit::slotChanged()
{
    if (d->maxLength <= 0)
    {
        return;
    }

    QToolTip::showText(mapToGlobal(QPoint(0, (-1)*(height() + 16))),
                       i18np("%1 character left", "%1 characters left",
                       maxLength() - text().size()),
                       this);
}

QSize DPlainTextEdit::sizeHint() const
{
    // NOTE: inspired from QLineEdit::sizeHint()

    QFontMetrics fm(font());
    QSize sizeHint     = QPlainTextEdit::sizeHint();
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
    qreal tm           = document()->documentMargin();
    int h              = qMax(fm.height(), qMax(16, iconSize)) * d->lines +
                         2 * frameWidth()                                 +
                         2 * tm;

    return QSize(sizeHint.width(), h);
}

} // namespace Digikam
