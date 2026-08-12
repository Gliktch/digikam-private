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

#pragma once

#include "dtextedit.h"
#include "digikam_config.h"

// Qt includes

#include <QMargins>
#include <QColor>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QToolButton>
#include <QMimeData>
#include <QIcon>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QScrollBar>
#include <QUrl>
#include <QToolTip>
#include <QEvent>

// KDE includes

#ifdef HAVE_SONNET

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wnonportable-include-path"
#   endif


#       include <sonnet/spellcheckdecorator.h>
#       include <sonnet/highlighter.h>

using namespace Sonnet;

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "localizesettings.h"
#include "localizecontainer.h"

namespace Digikam
{

class Q_DECL_HIDDEN DTextEditClearButton : public QToolButton
{
    Q_OBJECT

public:

    explicit DTextEditClearButton(QWidget* const parent);

};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN DTextEdit::Private
{
public:

    Private() = default;

    /**
     * Init the text widget with the spell-checker engine (optional).
     */
    void init(DTextEdit* const parent);

public:

#ifdef HAVE_SONNET

    Sonnet::SpellCheckDecorator* spellChecker = nullptr;

#endif

    QString                      ignoredMask;               ///< Mask of ignored characters in text editor.
    QString                      acceptedMask;              ///< Mask of accepted characters in text editor.

    unsigned int                 lines          = 3;

    DTextEditClearButton*        clrBtn         = nullptr;

    LocalizeContainer            container;                 ///< Spell checking settings container.

    int                          maxLength      = 0;

    bool                         clearBtnEnable = true;
};

// ------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DPlainTextEdit::Private
{
public:

    Private() = default;

    /**
     * Init the text widget with the spell-checker engine (optional).
     */
    void init(DPlainTextEdit* const parent);

public:

#ifdef HAVE_SONNET

    Sonnet::SpellCheckDecorator* spellChecker = nullptr;

#endif

    QString                      ignoredMask;               ///< Mask of ignored characters in text editor.
    QString                      acceptedMask;              ///< Mask of accepted characters in text editor.

    unsigned int                 lines          = 3;

    DTextEditClearButton*        clrBtn         = nullptr;

    LocalizeContainer            container;                 ///< Spell checking settings container.

    int                          maxLength      = 0;

    bool                         clearBtnEnable = true;
};

} // namespace Digikam
