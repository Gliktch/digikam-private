/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "altlangstredit.h"

// Qt includes

#include <QEvent>
#include <QIcon>
#include <QLineEdit>
#include <QFontMetrics>
#include <QRect>
#include <QMenu>
#include <QListWidget>
#include <QWidgetAction>
#include <QStyle>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QApplication>
#include <QComboBox>
#include <QScrollBar>
#include <QListWidgetItem>
#include <QMessageBox>

// KDE includes

#include <klazylocalizedstring.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "donlinetranslator.h"
#include "localizesettings.h"
#include "localizeselector.h"
#include "dexpanderbox.h"
#include "digikam_globals.h"

namespace Digikam
{

typedef QMap<QString, KLazyLocalizedString> LanguageCodeMap;

extern const LanguageCodeMap s_rfc3066ForXMP;

// ----------------------------------------------------------------------

class Q_DECL_HIDDEN AltLangStrEdit::Private
{
public:

    Private()  = default;
    ~Private() = default;

public:

    QString                        currentLanguage  = QLatin1String("x-default");

    uint                           linesVisible     = 0;

    QGridLayout*                   grid             = nullptr;

    QWidget*                       titleWidget      = nullptr;

    QToolButton*                   delValueButton   = nullptr;
    LocalizeSelector*              localizeSelector = nullptr;

    DTextEdit*                     valueEdit        = nullptr;

    QComboBox*                     languageCB       = nullptr;
    DOnlineTranslator*             trengine         = nullptr;
    QString                        trCode;

    MetaEngine::AltLangMap         values;
};

} // namespace Digikam
