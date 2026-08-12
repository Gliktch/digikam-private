/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmisc_p.h"

namespace Digikam
{

void SetupMisc::setupAppearance()
{
    // -- Application Appearance Options --------------------------------------------------------

    QWidget* const appearancePanel = new QWidget(d->tab);
    QGridLayout* const layout2     = new QGridLayout(appearancePanel);

    d->showSplashCheck             = new QCheckBox(i18n("&Show splash screen at startup"), appearancePanel);
    d->useNativeFileDialogCheck    = new QCheckBox(i18n("Use native file dialogs from system"), appearancePanel);
    d->drawFramesToGroupedCheck    = new QCheckBox(i18n("Draw frames around grouped items"), appearancePanel);

    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), appearancePanel);
    d->sidebarType            = new QComboBox(appearancePanel);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebar tab titles are visible. "
                                    "Use \"Only For Active Tab\" option if you use a small screen resolution as with a laptop computer."));

    d->applicationStyleLabel  = new QLabel(i18n("Widget style:"), appearancePanel);
    d->applicationStyle       = new QComboBox(appearancePanel);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));
    const auto styles         = QStyleFactory::keys();

    for (const QString& style : styles)
    {
        QString sitem = style;
        sitem[0]      = sitem[0].toUpper();

        if (sitem != QLatin1String("Macintosh"))        // See bug #475572
        {
            d->applicationStyle->addItem(sitem, sitem.toLower());
        }
    }

#ifndef HAVE_APPSTYLE_SUPPORT

    // See Bug #365262

    d->applicationStyleLabel->setVisible(false);
    d->applicationStyle->setVisible(false);

#endif

    d->applicationIconLabel    = new QLabel(i18n("Icon theme (changes after restart):"), appearancePanel);
    d->applicationIcon         = new QComboBox(appearancePanel);
    d->applicationIcon->setToolTip(i18n("Set this option to choose the default icon theme."));

    QMap<QString, QString> iconThemes;
    QMap<QString, QString> themeWhiteList;
    themeWhiteList.insert(QLatin1String("adwaita"),         i18nc("icon theme", "Adwaita"));
    themeWhiteList.insert(QLatin1String("breeze"),          i18nc("icon theme", "Breeze"));
    themeWhiteList.insert(QLatin1String("breeze-dark"),     i18nc("icon theme", "Breeze Dark"));
    themeWhiteList.insert(QLatin1String("faenza"),          i18nc("icon theme", "Faenza"));
    themeWhiteList.insert(QLatin1String("faenza-ambiance"), i18nc("icon theme", "Ambiance"));
    themeWhiteList.insert(QLatin1String("humanity"),        i18nc("icon theme", "Humanity"));
    themeWhiteList.insert(QLatin1String("oxygen"),          i18nc("icon theme", "Oxygen"));
    const auto paths = QIcon::themeSearchPaths();

    for (const QString& path : paths)
    {
        QDirIterator it(path, QDir::Dirs       |
                              QDir::NoSymLinks |
                              QDir::NoDotAndDotDot);

        while (it.hasNext())
        {
            if (QFile::exists(it.next() + QLatin1String("/index.theme")))
            {
                QString iconKey = it.fileInfo().fileName().toLower();

                if (themeWhiteList.contains(iconKey))
                {
                    iconThemes[themeWhiteList.value(iconKey)] = it.fileInfo().fileName();
                }
            }
        }
    }

    QMap<QString, QString>::const_iterator it = iconThemes.constBegin();
    d->applicationIcon->addItem(i18n("Use Icon Theme From System"), QString());

    for ( ; it != iconThemes.constEnd() ; ++it)
    {
        d->applicationIcon->addItem(it.key(), it.value());
    }


    d->dateTimeFormatLabel    = new QLabel(i18n("Date and time format (%1):",
                                                getDateFormatLinkText()), appearancePanel);
    d->dateTimeFormatLabel->setOpenExternalLinks(true);
    d->dateTimeFormatLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

    DHBox* const dtFormatHBox = new DHBox(appearancePanel);
    d->dateTimeFormatEdit     = new QLineEdit(dtFormatHBox);
    d->dateTimeFormatEdit->setToolTip(i18n("This string is used as the date and time format in whole application."));
    d->dateTimeFormatReset    = new QToolButton(dtFormatHBox);
    d->dateTimeFormatReset->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->dateTimeFormatReset->setToolTip(i18n("Reverts to the local short date and time format"));

    d->applicationFont        = new DFontSelect(i18n("Application font:"), appearancePanel);
    d->applicationFont->setToolTip(i18n("Select here the font used to display text in whole application."));

    // --------------------------------------------------------

    layout2->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    layout2->setSpacing(d->spacing);
    layout2->addWidget(d->showSplashCheck,          0, 0, 1, 2);
    layout2->addWidget(d->useNativeFileDialogCheck, 1, 0, 1, 2);
    layout2->addWidget(d->drawFramesToGroupedCheck, 2, 0, 1, 2);
    layout2->addWidget(d->sidebarTypeLabel,         3, 0, 1, 1);
    layout2->addWidget(d->sidebarType,              3, 1, 1, 1);
    layout2->addWidget(d->applicationStyleLabel,    4, 0, 1, 1);
    layout2->addWidget(d->applicationStyle,         4, 1, 1, 1);
    layout2->addWidget(d->applicationIconLabel,     5, 0, 1, 1);
    layout2->addWidget(d->applicationIcon,          5, 1, 1, 1);
    layout2->addWidget(d->dateTimeFormatLabel,      6, 0, 1, 1);
    layout2->addWidget(dtFormatHBox,                6, 1, 1, 1);
    layout2->addWidget(d->applicationFont,          7, 0, 1, 2);
    layout2->setColumnStretch(1, 10);
    layout2->setRowStretch(8, 10);

    d->tab->insertTab(Appearance, appearancePanel, i18nc("@title:tab", "Appearance"));

    connect(d->dateTimeFormatReset, &QToolButton::clicked,
            this, [this]()
        {
            d->dateTimeFormatEdit->setText(QLocale().dateTimeFormat(QLocale::ShortFormat));
        }
    );
}

} // namespace Digikam
