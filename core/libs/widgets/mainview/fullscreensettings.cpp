/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen settings widget
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fullscreensettings.h"

// Qt includes

#include <QString>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN FullScreenSettings::Private
{
public:

    Private() = default;

    int        options          = FS_NONE;

    QCheckBox* hideToolBars     = nullptr;
    QCheckBox* hideThumbBar     = nullptr;
    QCheckBox* hideSideBars     = nullptr;
    QCheckBox* hideStatusBar    = nullptr;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QGroupBox(i18n("Full-screen Options"), parent),
      d        (new Private)
{
    const int spacing        = layoutSpacing();


    d->options               = options;
    QVBoxLayout* const vlay  = new QVBoxLayout(this);
    d->hideToolBars          = new QCheckBox(i18n("H&ide toolbars"),  this);
    d->hideToolBars->setWhatsThis(i18n("Hide all toolbars when window switch in full-screen mode."));

    d->hideThumbBar          = new QCheckBox(i18n("Hide &thumbbar"),  this);
    d->hideThumbBar->setWhatsThis(i18n("Hide thumbbar view when window switch in full-screen mode."));

    d->hideSideBars          = new QCheckBox(i18n("Hide &sidebars"),  this);
    d->hideSideBars->setWhatsThis(i18n("Hide all side-bars when window switch in full-screen mode."));

    d->hideStatusBar         = new QCheckBox(i18n("Hide st&atusbar"), this);
    d->hideStatusBar->setWhatsThis(i18n("Hide statusbar when window switch in full-screen mode."));

    vlay->addWidget(d->hideToolBars);
    vlay->addWidget(d->hideThumbBar);
    vlay->addWidget(d->hideSideBars);
    vlay->addWidget(d->hideStatusBar);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(0);

    if (!(options & FS_TOOLBARS))
    {
        d->hideToolBars->hide();
    }

    if (!(options & FS_THUMBBAR))
    {
        d->hideThumbBar->hide();
    }

    if (!(options & FS_SIDEBARS))
    {
        d->hideSideBars->hide();
    }

    if (!(options & FS_STATUSBAR))
    {
        d->hideStatusBar->hide();
    }
}

FullScreenSettings::~FullScreenSettings()
{
    delete d;
}

void FullScreenSettings::readSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
    {
        d->hideToolBars->setChecked(group.readEntry(DXmlGuiWindow::configFullScreenHideToolBarsEntry(),   false));
    }

    if (d->options & FS_THUMBBAR)
    {
        d->hideThumbBar->setChecked(group.readEntry(DXmlGuiWindow::configFullScreenHideThumbBarEntry(),   true));
    }

    if (d->options & FS_SIDEBARS)
    {
        d->hideSideBars->setChecked(group.readEntry(DXmlGuiWindow::configFullScreenHideSideBarsEntry(),   false));
    }

    if (d->options & FS_STATUSBAR)
    {
        d->hideStatusBar->setChecked(group.readEntry(DXmlGuiWindow::configFullScreenHideStatusBarEntry(), false));
    }
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
    {
        group.writeEntry(DXmlGuiWindow::configFullScreenHideToolBarsEntry(),  d->hideToolBars->isChecked());
    }

    if (d->options & FS_THUMBBAR)
    {
        group.writeEntry(DXmlGuiWindow::configFullScreenHideThumbBarEntry(),  d->hideThumbBar->isChecked());
    }

    if (d->options & FS_SIDEBARS)
    {
        group.writeEntry(DXmlGuiWindow::configFullScreenHideSideBarsEntry(),  d->hideSideBars->isChecked());
    }

    if (d->options & FS_STATUSBAR)
    {
        group.writeEntry(DXmlGuiWindow::configFullScreenHideStatusBarEntry(), d->hideStatusBar->isChecked());
    }

    group.sync();
}

} // namespace Digikam

#include "moc_fullscreensettings.cpp"
