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

DTextEditClearButton::DTextEditClearButton(QWidget* const parent)
    : QToolButton(parent)
{
    setStyleSheet(QLatin1String("QToolButton { border: none; padding: 0px; }"));
    QIcon icon = qApp->style()->standardIcon(QStyle::SP_LineEditClearButton);
    setFocusPolicy(Qt::NoFocus);
    setIcon(icon);
}

// -------------------------------------------------------------------------------

void DTextEdit::Private::init(DTextEdit* const parent)
{

#ifdef HAVE_SONNET

    spellChecker                   = new SpellCheckDecorator(parent);

    // Auto-detect language enabled by default.

    spellChecker->highlighter()->setAutoDetectLanguageDisabled(false);

    LocalizeSettings* const config = LocalizeSettings::instance();

    if (config)
    {
        parent->setLocalizeSettings(config->settings());

        QObject::connect(config, &LocalizeSettings::signalSettingsChanged,
                         parent, [=]()
            {
                parent->setLocalizeSettings(config->settings());
            }
        );
    }

#endif

    parent->setLinesVisible(lines);
    parent->setTabChangesFocus(true);

    clrBtn                  = new DTextEditClearButton(parent);
    QHBoxLayout* const hlay = new QHBoxLayout(parent);
    hlay->addStretch(10);
    hlay->addWidget(clrBtn);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);
    clrBtn->hide();

    connect(clrBtn, &DTextEditClearButton::clicked,
            parent, &QTextEdit::clear);

    // Mimic QLineEdit

    QObject::connect(parent, &QTextEdit::textChanged,
                     parent, [this, parent]()
        {
            if (clearBtnEnable && (lines == 1))
            {
                clrBtn->setVisible(!parent->text().isEmpty());
            }
        }
    );

    QObject::connect(parent, &QTextEdit::textChanged,
                     parent, &DTextEdit::slotChanged);
}

// ------------------------------------------------------------------------------------------------

void DPlainTextEdit::Private::init(DPlainTextEdit* const parent)
{

#ifdef HAVE_SONNET

    spellChecker                   = new SpellCheckDecorator(parent);

    // Auto-detect language enabled by default.

    spellChecker->highlighter()->setAutoDetectLanguageDisabled(false);

    LocalizeSettings* const config = LocalizeSettings::instance();

    if (config)
    {
        parent->setLocalizeSettings(config->settings());

        QObject::connect(config, &LocalizeSettings::signalSettingsChanged,
                         parent, [parent, config]()
            {
                parent->setLocalizeSettings(config->settings());
            }
        );
    }

#endif

    parent->setLinesVisible(lines);

    clrBtn                  = new DTextEditClearButton(parent);
    QHBoxLayout* const hlay = new QHBoxLayout(parent);
    hlay->addStretch(10);
    hlay->addWidget(clrBtn);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);
    clrBtn->hide();

    connect(clrBtn, &DTextEditClearButton::clicked,
            parent, &QPlainTextEdit::clear);

    // Mimic QLineEdit

    QObject::connect(parent, &QPlainTextEdit::textChanged,
                     parent, [this, parent]()
        {
            if (clearBtnEnable && (lines == 1))
            {
                clrBtn->setVisible(!parent->text().isEmpty());
            }
        }
    );

    QObject::connect(parent, &QPlainTextEdit::textChanged,
                     parent, &DPlainTextEdit::slotChanged);
}

} // namespace Digikam

#include "moc_dtextedit_p.cpp"
