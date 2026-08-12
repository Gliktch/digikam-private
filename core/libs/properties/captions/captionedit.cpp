/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : caption editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "captionedit.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN CaptionEdit::Private
{
public:

    Private() = default;

    AltLangStrEdit* altLangStrEdit  = nullptr;

    QLineEdit*      authorEdit      = nullptr;

    CaptionsMap     captionsValues;

    QString         lastDeletedLanguage;
    CaptionValues   lastDeletedValues;
};

CaptionEdit::CaptionEdit(QWidget* const parent)
    : DVBox(parent),
      d    (new Private)
{
    d->altLangStrEdit = new AltLangStrEdit(this, 0);
    d->altLangStrEdit->setTitle(i18n("Captions: "));

    d->authorEdit     = new QLineEdit(this);
    d->authorEdit->setClearButtonEnabled(true);
    d->authorEdit->setPlaceholderText(i18n("Enter caption author name here."));

    setContentsMargins(QMargins());
    setSpacing(0);

    connect(d->altLangStrEdit, SIGNAL(signalSelectionChanged(QString)),
            this, SLOT(slotSelectionChanged(QString)));

    connect(d->altLangStrEdit, SIGNAL(signalChanged(QString,QString)),
            this, SLOT(slotCaptionModified(QString,QString)));

    connect(d->altLangStrEdit, SIGNAL(signalValueAdded(QString,QString)),
            this, SLOT(slotAddValue(QString,QString)));

    connect(d->altLangStrEdit, SIGNAL(signalValueDeleted(QString)),
            this, SLOT(slotDeleteValue(QString)));

    connect(d->authorEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotAuthorChanged(QString)));
}

CaptionEdit::~CaptionEdit()
{
    delete d;
}

void CaptionEdit::reset()
{
    d->altLangStrEdit->reset();

    d->authorEdit->blockSignals(true);
    d->authorEdit->clear();
    d->authorEdit->blockSignals(false);

    d->captionsValues.clear();
}

void CaptionEdit::setPlaceholderText(const QString& msg)
{
    d->altLangStrEdit->setPlaceholderText(msg);
}

QString CaptionEdit::currentLanguageCode() const
{
    return d->altLangStrEdit->currentLanguageCode();
}

void CaptionEdit::setCurrentLanguageCode(const QString& lang)
{
    d->altLangStrEdit->setCurrentLanguageCode(lang);
}

void CaptionEdit::slotAddValue(const QString& lang, const QString& text)
{
    CaptionValues val;
    val.caption = text;
    val.author  = d->authorEdit->text();
    val.date    = QDateTime::currentDateTime();

    // The user may have removed the text and directly entered a new one. Do not drop author then.

    if (val.author.isEmpty() && d->lastDeletedLanguage == lang)
    {
        val.author = d->lastDeletedValues.author;
        d->authorEdit->blockSignals(true);
        d->authorEdit->setText(val.author);
        d->authorEdit->blockSignals(false);
    }

    d->lastDeletedLanguage.clear();
    d->captionsValues.insert(lang, val);

    Q_EMIT signalModified();
}

void CaptionEdit::slotCaptionModified(const QString& lang, const QString& text)
{
    slotAddValue(lang, text);
}

void CaptionEdit::slotDeleteValue(const QString& lang)
{
    d->lastDeletedLanguage = lang;
    d->lastDeletedValues   = d->captionsValues.value(lang);

    d->captionsValues.remove(lang);
    d->authorEdit->blockSignals(true);
    d->authorEdit->clear();
    d->authorEdit->blockSignals(false);

    Q_EMIT signalModified();
}

void CaptionEdit::slotSelectionChanged(const QString& lang)
{
    QString author = d->captionsValues.value(lang).author;
    d->authorEdit->blockSignals(true);
    d->authorEdit->setText(author);
    d->authorEdit->blockSignals(false);
}

void CaptionEdit::setValues(const CaptionsMap& values)
{
    d->lastDeletedLanguage.clear();

    d->captionsValues = values;
    d->altLangStrEdit->setValues(d->captionsValues.toAltLangMap());
    slotSelectionChanged(d->altLangStrEdit->currentLanguageCode());
}

CaptionsMap& CaptionEdit::values() const
{
    return d->captionsValues;
}

void CaptionEdit::slotAuthorChanged(const QString& text)
{
    CaptionValues captionValues = d->captionsValues.value(d->altLangStrEdit->currentLanguageCode());

    if (text != captionValues.author)
    {
        d->altLangStrEdit->addCurrent();
    }
}

AltLangStrEdit* CaptionEdit::altLangStrEdit() const
{
    return d->altLangStrEdit;
}

QLineEdit* CaptionEdit::authorEdit() const
{
    return d->authorEdit;
}

} // namespace Digikam

#include "moc_captionedit.cpp"
