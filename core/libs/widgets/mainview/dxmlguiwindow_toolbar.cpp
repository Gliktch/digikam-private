/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window - Tool-bar methods.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dxmlguiwindow_p.h"

namespace Digikam
{

void DXmlGuiWindow::slotConfToolbars()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    saveMainWindowSettings(group);

    QPointer<KEditToolBar> dlg = new KEditToolBar(factory(), this);

    QDialogButtonBox* const btnbox = dlg->findChild<QDialogButtonBox*>();

    if (btnbox)
    {
        btnbox->setStandardButtons(btnbox->standardButtons() | QDialogButtonBox::Help);

        connect(btnbox->button(QDialogButtonBox::Help), &QPushButton::clicked,
                this, []()
            {
                openOnlineDocumentation(QLatin1String("setup_application"),
                                        QLatin1String("toolbars_settings"));
            }
        );
    }

    connect(dlg, &KEditToolBar::newToolBarConfig,
            this, &DXmlGuiWindow::slotNewToolbarConfig);

    (void)dialogExec(dlg);
    delete dlg;
}

void DXmlGuiWindow::slotNewToolbarConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    applyMainWindowSettings(group);
}

KToolBar* DXmlGuiWindow::mainToolBar() const
{
    return const_cast<DXmlGuiWindow*>(this)->toolBar(QLatin1String("mainToolBar"));
}

void DXmlGuiWindow::showToolBars(bool visible)
{
    // We will hide toolbars: store previous state for future restoring.

    if (!visible)
    {
        d->toolbarsVisibility.clear();
        const auto bars = toolBars();

        for (KToolBar* const toolbar : bars)
        {
            if (toolbar)
            {
                bool visibility = toolbar->isVisible();
                d->toolbarsVisibility.insert(toolbar, visibility);
            }
        }
    }

    // Switch toolbars visibility

    for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin() ;
         it != d->toolbarsVisibility.constEnd() ; ++it)
    {
        KToolBar* const toolbar = it.key();
        bool visibility         = it.value();

        if (toolbar)
        {
            if (visible && visibility)
            {
                toolbar->show();
            }
            else
            {
                toolbar->hide();
            }
        }
    }

    // We will show toolbars: restore previous state.

    if (visible)
    {
        for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin() ;
             it != d->toolbarsVisibility.constEnd() ; ++it)
        {
            KToolBar* const toolbar = it.key();
            bool visibility         = it.value();

            if (toolbar)
            {
                visibility ? toolbar->show()
                           : toolbar->hide();
            }
        }
    }
}

} // namespace Digikam
